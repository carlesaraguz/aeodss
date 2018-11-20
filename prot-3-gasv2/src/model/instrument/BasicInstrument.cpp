/***********************************************************************************************//**
 *  Conical, nadir-pointing instrument.
 *  @class      BasicInstrument
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *              Marc Closa (MC), marc.closa.tarres@alu-etsetb.upc.edu
 *  @date       2018-nov-15
 *  @version    0.2
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "BasicInstrument.hpp"

CREATE_LOGGER(BasicInstrument)

BasicInstrument::BasicInstrument(void)
    : BasicInstrument(Random::getUf(Config::agent_aperture_min, Config::agent_aperture_max))
{ }

BasicInstrument::BasicInstrument(float aperture)
    : m_aperture(std::fmod(aperture, 180.f))
    , m_swath(-1.f)
    , m_energy_rate(Random::getUf(Config::instrument_energy_min, Config::instrument_energy_max))
    , m_storage_rate(Random::getUf(Config::instrument_storage_min, Config::instrument_storage_max))
    , m_enabled(false)
{
    if(aperture > 180.f) {
        Log::warn << "Instrument constructed with an aperture of " << aperture
            << " (> 180º). Fixed and set at " << m_aperture << ".\n";
    }
}

void BasicInstrument::setPosition(sf::Vector3f p)
{
    m_position = p;
    if(m_swath == -1.f && Config::motion_model == AgentMotionType::ORBITAL) {
        m_swath = getSwath(p, m_aperture);
        Log::dbg << "Instrument has a true swath of " << m_swath / 1e3 << " km. (aperture is " << m_aperture << "º)\n";
    } else if(m_swath == -1.f && Config::motion_model != AgentMotionType::ORBITAL) {
        m_swath = m_aperture;
        Log::dbg << "Instrument has a swath/aperture of " << m_swath << " (2-d motion model).\n";
    }
}

bool BasicInstrument::applyToDistance2D(
    unsigned int ox,
    unsigned int oy,
    float r,
    bool world_distance,
    std::function<void(unsigned int, unsigned int)> f) const
{
    /*  Find other cells that are at a distance of `r` from (ox, oy):
     *    - ox, oy : Spiral origin/offset.
     *    - xx, yy : Spiral iterators (origin = 0,0).
     *    - dx, dy : Iterator steps.
     **/
    int xx, yy, dx, dy, aux;
    xx = yy = dx = 0;
    dy = -1;
    int b;
    if(world_distance) {
        b = 2 * (std::ceil(r) + 1);
    } else {
        b = 2 * std::ceil(std::max(r / m_env_info.rw, r / m_env_info.rh) + 1);
    }
    int max_iter = b * b;
    bool at_r = false;
    int corner_count = 0;
    int i = 0;
    int span_hor, span_ver;
    if(world_distance) {
        span_hor = World::getWidth();
        span_ver = World::getHeight();
    } else {
        span_hor = m_env_info.mw;
        span_ver = m_env_info.mh;
    }
    for(; i < max_iter; i++) {
        if( (xx + (int)ox < span_hor) &&
            (xx + (int)ox >= 0)       &&
            (yy + (int)oy < span_ver) &&
            (yy + (int)oy >= 0)
        ) {
            float r1, r2;
            if(world_distance) {
                r1 = xx;
                r2 = yy;
            } else {
                r1 = xx * m_env_info.rw;
                r2 = yy * m_env_info.rh;
            }
            float dist = std::sqrt(r1 * r1 + r2 * r2);
            if(dist <= r) {
                f(xx + ox, yy + oy);
                at_r = true;
                corner_count = 0;
            }
            if(corner_count >= 4 && !at_r) {
                break;
            } else if(corner_count >= 4) {
                at_r = false;
            }
        }
        if((xx == yy) || ((xx < 0) && (xx == -yy)) || ((xx > 0) && (xx == 1 - yy))) {
            aux  = dx;
            dx = -dy;
            dy = aux;
            corner_count++;
        }
        xx += dx;
        yy += dy;
    }
    return (i < max_iter);
}

void BasicInstrument::applyToDistance3D(
    unsigned int ox,
    unsigned int oy,
    sf::Vector3f p,
    double t,
    float r,
    bool world_cells,
    std::function<void(unsigned int, unsigned int)> f,
    const std::vector<std::vector<sf::Vector3f> >& lut) const
{
    /*  Get position in ECEF to be able to compare it with the pre-computed positions in the LUT.
     *  This is faster than computing geographic to ECI transformations for every iterated point.
     *  The implemented solution relies on the existance of a pre-computed LUT, which is less
     *  memory-efficient but still reasonable. Note that a single LUT for a 180x90 model takes
     *  roughly 190 KiB (per Agent), whereas a 1800x900 model takes roughly 18.5 MiB.
     **/
    auto p_ecef = CoordinateSystemUtils::fromECIToECEF(p, t);
    int span_hor, span_ver, xx, yy, its = 0;
    if(world_cells) {
        span_hor = World::getWidth();
        span_ver = World::getHeight();
    } else {
        span_hor = m_env_info.mw;
        span_ver = m_env_info.mh;
    }
    bool at_r = false;
    auto check_cell = [&](int xit, int yit, bool verbose = false) {
        auto s_ecef = lut.at(xit).at(yit);
        float dist = MathUtils::norm(p_ecef - s_ecef);
        if(dist <= r) {
            f(xit, yit);
            at_r = true;
        }
        if(verbose) {
            Log::dbg << "Point (" << xit << ", " << yit << ") is at distance " << dist << " (> " << r << ").\n";
            Log::dbg << "  p = (" << p_ecef.x << ", " << p_ecef.y << ", " << p_ecef.z << ").\n";
            Log::dbg << "  s = (" << s_ecef.x << ", " << s_ecef.y << ", " << s_ecef.z << ").\n";
        }
    };

    check_cell(ox, oy);
    if(!at_r) {
        check_cell(ox, oy, true);
        auto o_eci = CoordinateSystemUtils::fromECEFToECI(lut.at(ox).at(oy), t);
        Log::warn << "Position in ECI = (" << p.x << ", " << p.y << ", " << p.z << ").\n";
        Log::warn << "Origin   in ECI = (" << o_eci.x << ", " << o_eci.y << ", " << o_eci.z << ").\n";
        Log::warn << "Error: " << MathUtils::norm(p - o_eci) << ".\n";
    }

    /* Iterate for every quadrant until no more cells are found horizontally. */
    for(int quadrant = 0; quadrant < 4; quadrant++) {
        switch(quadrant) {
            case 0:
                for(yy = oy; yy < span_ver; yy++) {
                    at_r = false;
                    for(xx = ox; xx < span_hor; xx++) {
                        check_cell(xx, yy);
                    }
                    if(!at_r) {
                        break;
                    }
                }
                break;
            case 1:
                for(yy = oy; yy >= 0; yy--) {
                    at_r = false;
                    for(xx = ox; xx < span_hor; xx++) {
                        check_cell(xx, yy);
                    }
                    if(!at_r) {
                        break;
                    }
                }
                break;
            case 2:
                for(yy = oy; yy < span_ver; yy++) {
                    at_r = false;
                    for(xx = ox; xx >= 0; xx--) {
                        its++;
                        check_cell(xx, yy);
                    }
                    if(!at_r) {
                        break;
                    }
                }
                break;
            case 3:
                for(yy = oy; yy >= 0; yy--) {
                    at_r = false;
                    for(xx = ox; xx >= 0; xx--) {
                        its++;
                        check_cell(xx, yy);
                    }
                    if(!at_r) {
                        break;
                    }
                }
                break;
        }
    }
}

std::vector<sf::Vector2i> BasicInstrument::getVisibleCells(
    const std::vector<std::vector<sf::Vector3f> >& lut,
    float dist, sf::Vector3f position,
    bool world_cells,
    double t
) const
{
    int ox, oy;
    std::vector<sf::Vector2i> cells;
    auto f = [&cells](unsigned int x, unsigned int y) {
        cells.push_back(sf::Vector2i(x, y));
    };
    if(Config::motion_model != AgentMotionType::ORBITAL) {
        if(world_cells) {
            ox = std::round(position.x);
            oy = std::round(position.y);
            if(ox > (int)Config::world_width || oy > (int)Config::world_height || ox < 0 || oy < 0) {
                Log::err << "Can't get visible cells for a point that is outside the world space O(" << ox << ", " << oy << ")\n";
                throw std::runtime_error("Can't compute visible cells if position is outside the environment space.");
            }
        } else {
            ox = std::round(position.x / m_env_info.rw);
            oy = std::round(position.y / m_env_info.rh);
            if(ox > (int)m_env_info.mw || oy > (int)m_env_info.mh || ox < 0 || oy < 0) {
                Log::err << "Can't get visible cells for a point that is outside the model space O(" << ox << ", " << oy << ")\n";
                throw std::runtime_error("Can't compute visible cells if position is outside the environment space.");
            }
        }
        if(!applyToDistance2D(ox, oy, dist, world_cells, f)) {
            Log::err << "Failed to get visible (" << (world_cells ? "world" : "model") << ") cells "
                << "from P = (" << position.x << ", " << position.y << ").\n";
            Log::err << "Origin of spiral iterator set to (" << ox << ", "<< oy << "). Looking for cells at a distances of " << dist << ".\n";
            Log::err << "Maximum number of operations reached. Abosrting program.\n";
            throw std::runtime_error("Failed to get visible (model) cells from the given position. Maximum number of operations reached.");
        }
    } else {
        if(t <= -1.0) {
            t = VirtualTime::now();
        }
        auto proj = AgentMotion::getProjection2D(position, t);
        if(world_cells) {
            ox = proj.x;
            oy = proj.y;
        } else {
            ox = (int)std::round(proj.x / m_env_info.rw) % m_env_info.mw;
            oy = (int)std::round(proj.y / m_env_info.rh) % m_env_info.mh;
        }
        applyToDistance3D(ox, oy, position, t, dist, world_cells, f, lut);
    }
    if(cells.size() == 0) {
        Log::err << "Could not find visible (" << (world_cells ? "world" : "model") << ") cells at distance " << dist << ".\n";
        Log::err << "   Will provide one (i.e. the projected position) and try to continue.\n";
        cells.push_back(sf::Vector2i(ox, oy));
    }
    return cells;
}

std::vector<sf::Vector2i> BasicInstrument::getVisibleCells(float dist, sf::Vector2f position, bool world_cells) const
{
    if(Config::motion_model == AgentMotionType::ORBITAL) {
        Log::err << "Computing visible cells with a 2-d function. Will result in unexpected behaviour.\n";
    }
    return getVisibleCells({ }, dist, {position.x, position.y, 0.f}, world_cells);
}

std::vector<sf::Vector2i> BasicInstrument::getVisibleCells(
    const std::vector<std::vector<sf::Vector3f> >& lut,
    bool world_cells
) const
{
    if(m_swath == -1.f) {
        Log::err << "Computing visible cells without a valid swath (and position).\n";
    }
    if(Config::motion_model == AgentMotionType::ORBITAL) {
        return getVisibleCells(lut, getSlantRangeAt(m_aperture / 2.f, m_position), m_position, world_cells);
    } else {
        return getVisibleCells(lut, m_swath, m_position, world_cells);
    }
}

float BasicInstrument::getSlantRangeAt(float deg, sf::Vector3f p) const
{
    /*  NOTE:
     *  Precise computation of slant angles should consider the actual radius of the Earth at the
     *  point of observation. Since we are not interested in extremely precise figures nor want to
     *  perform geographic corrections at this point, we'll assume that the radius of the Earth is
     *  constant and equal to its equatorial value (i.e. semi-major axis of WGS-86 ellipsoid).
     **/
    float ang_rad = MathUtils::degToRad(std::fmod(deg, 180.f));
    float h = MathUtils::norm(p);
    float lambda = Config::pi - std::asin((h / Config::earth_wgs84_a) * std::sin(ang_rad));
    float alpha  = Config::pi - lambda - ang_rad;
    float sr = Config::earth_wgs84_a * std::sin(alpha) / std::sin(ang_rad);
    if(std::isnan(sr) || sr <= 1.f) {
        Log::err << "Computing slant range gave \'" << sr << "\' for instrument at " << deg << "º\n";
        Log::err << "  h = " << h << " meters.\n";
        Log::err << "  (h/R)·sin(ẟ) = " << (h / Config::earth_wgs84_a) * std::sin(ang_rad) << ".\n";
        Log::err << "  ẟ = " << deg << "º = " << ang_rad << " rad.\n";
        Log::err << "  λ = " << MathUtils::radToDeg(lambda) << "º = " << lambda << "rad.\n";
        Log::err << "  α = " << MathUtils::radToDeg(alpha) << "º = " << alpha << "rad.\n";
        Log::err << "  sin(α) = " << std::sin(alpha) << ".\n";
        Log::err << "  sin(ẟ) = " << std::sin(ang_rad) << ".\n";
        throw std::runtime_error("Error computing slant range.");
    }
    return sr;
}

float BasicInstrument::getSwath(sf::Vector3f p, float aperture) const
{
    if(Config::motion_model != AgentMotionType::ORBITAL) {
        Log::warn << "Attempting to compute instrument swath for a 2-d motion model.\n";
    }
    float ang_rad = MathUtils::degToRad(std::fmod(aperture, 180.f)) / 2.f;
    float h = MathUtils::norm(p);
    float lambda = Config::pi - std::asin((h / Config::earth_wgs84_a) * std::sin(ang_rad));
    float alpha  = Config::pi - lambda - ang_rad;
    return 2.f * alpha * Config::earth_wgs84_a;
}

std::vector<sf::Vector2f> BasicInstrument::getFootprint(void) const
{
    int n_points;
    std::vector<sf::Vector2f> footprint;
    switch(Config::motion_model){
        case AgentMotionType::LINEAR_BOUNCE:
        case AgentMotionType::LINEAR_INFINITE:
        case AgentMotionType::SINUSOIDAL:
            {
                n_points = 16;
                float da = 2.f * Config::pi / (float)n_points;
                for(int i = 0; i <= n_points; i++) {
                    footprint.push_back(((m_swath / 2.f) * sf::Vector2f(std::cos(da * (float)i), std::sin(da * (float)i))));
                }
            }
            break;

        case AgentMotionType::ORBITAL:
            {
                n_points = 20;
                float R = Config::earth_wgs84_a;
                sf::Vector3f p = m_position;

                float aperture = MathUtils::degToRad(getSwath());

                float h = MathUtils::norm(p);

                float lambda = Config::pi - std::asin((h / R) * std::sin(aperture / 2));
                float alpha  = Config::pi - lambda - aperture / 2;
                float r = R * std::sin(alpha) / std::sin(aperture / 2);

                /* Also can be used:
                 * float footprint_radius = R * std::sin(alpha);
                 **/
                float footprint_radius = r * std::sin(aperture / 2);

                float c_len = R * cos(alpha);

                /* Computation of cross product */
                sf::Vector3f p_n = p / h;
                sf::Vector3f v = sf::Vector3f(p_n.x, p_n.y, p_n.z);
                sf::Vector3f c = v * c_len;

                /* When v = [1 0 0], a will be a = [0 0 0] using our perpendicular vector generator method.
                 * For this reason, we check that v = [1 0 0] and in this case we force a = [0 1 0] that is
                 * always perpendicular to v. And then, we generate b with the cross product.
                 **/

                // sf::Vector3f a = sf::Vector3f(v.z, v.z, (-v.x - v.y));
                sf::Vector3f a = sf::Vector3f(0, v.z, -v.y);
                a = MathUtils::makeUnitary(a);

                if(a == sf::Vector3f(0, 0, 0)) {
                    // a = sf::Vector3f((-v.y - v.z), v.x, v.x);
                    a = sf::Vector3f(0, 1, 0);
                }

                sf::Vector3f b = sf::Vector3f();

                b.x = v.y * a.z - v.z * a.y;
                b.y = v.x * a.z - v.z * a.x;
                b.z = v.x * a.y - v.y * a.x;

                b = MathUtils::makeUnitary(b);

                float step = 2.f * Config::pi / (float)n_points;

                std::vector<sf::Vector2f> positions;

                sf::Vector3f n_pole = sf::Vector3f(0, 0, R);
                sf::Vector3f s_pole = sf::Vector3f(0, 0, -R);

                bool north_included = false;
                bool south_included = false;

                float distance_to_north = std::sqrt((c.x - n_pole.x) * (c.x - n_pole.x) + (c.y - n_pole.y) * (c.y - n_pole.y)
                    + (c.z - n_pole.z) * (c.z - n_pole.z));
                float distance_to_south = std::sqrt((c.x - s_pole.x) * (c.x - s_pole.x) + (c.y - s_pole.y) * (c.y - s_pole.y)
                        + (c.z - s_pole.z) * (c.z - s_pole.z));

                if(distance_to_north < footprint_radius) {
                    north_included = true;
                } else if(distance_to_south < footprint_radius) {
                    south_included = true;
                }

                for(float th = 0; th < n_points; th += step) {
                    float footprint_x = c.x + footprint_radius * std::cos(th) * a.x + footprint_radius * std::sin(th) * b.x;
                    float footprint_y = c.z + footprint_radius * std::cos(th) * a.y + footprint_radius * std::sin(th) * b.y;
                    float footprint_z = c.z + footprint_radius * std::cos(th) * a.z + footprint_radius * std::sin(th) * b.z;

                    sf::Vector3f fp_pos = sf::Vector3f(footprint_x, footprint_y, footprint_z);
                    sf::Vector2f proj   = AgentMotion::getProjection2D(fp_pos, VirtualTime::now());
                    sf::Vector2f p_proj = AgentMotion::getProjection2D(p, VirtualTime::now());

                    // std::cout << "X: " << fp_pos.x << "; Y: " << fp_pos.y << "; Z: " << fp_pos.z << '\n';
                    std::cout << "X: " << proj.x << "; Y: " << proj.y <<'\n';
                    proj = p_proj - proj;
                    sf::Vector2f prev = sf::Vector2f();

                    if (positions.size() > 0) {
                        prev = positions.back();
                    }
                    north_included = false;
                    if(north_included) {
                        float border_y = std::abs(prev.y - proj.y);

                        if(prev.x > proj.x) {
                            positions.push_back(sf::Vector2f(Config::world_width, border_y));
                            positions.push_back(sf::Vector2f(Config::world_width, 0));
                            positions.push_back(sf::Vector2f(0, 0));
                            positions.push_back(sf::Vector2f(0, border_y));
                        } else {
                            positions.push_back(sf::Vector2f(0, border_y));
                            positions.push_back(sf::Vector2f(0, 0));
                            positions.push_back(sf::Vector2f(Config::world_width, 0));
                            positions.push_back(sf::Vector2f(Config::world_width, border_y));
                        }
                    }
                    south_included = false;
                    if(south_included) {
                        float border_y = std::abs(prev.y - proj.y) / 2;

                        if(prev.x > proj.x) {
                            positions.push_back(sf::Vector2f(Config::world_width, border_y));
                            positions.push_back(sf::Vector2f(Config::world_width, Config::world_height));
                            positions.push_back(sf::Vector2f(0, Config::world_height));
                            positions.push_back(sf::Vector2f(0, border_y));
                        } else {
                            positions.push_back(sf::Vector2f(0, border_y));
                            positions.push_back(sf::Vector2f(0, Config::world_height));
                            positions.push_back(sf::Vector2f(Config::world_width, Config::world_height));
                            positions.push_back(sf::Vector2f(Config::world_width, border_y));
                        }
                    }

                    bool is_splitted = false;

                    float x_distance = std::abs(proj.x - prev.x);

                    if(x_distance > Config::world_width / 2) {
                        is_splitted = true;
                    }
                    is_splitted = false;
                    if(is_splitted){
                        /* Compute border point */
                        float border_y = std::abs(prev.y - proj.y);

                        if(prev.x > proj.x) {
                            positions.push_back(sf::Vector2f(Config::world_width, border_y));
                            positions.push_back(sf::Vector2f(Config::world_width, Config::world_height));
                            positions.push_back(sf::Vector2f(0, Config::world_height));
                            positions.push_back(sf::Vector2f(0, border_y));
                        } else {
                            positions.push_back(sf::Vector2f(0, border_y));
                            positions.push_back(sf::Vector2f(0, 0));
                            positions.push_back(sf::Vector2f(Config::world_width, 0));
                            positions.push_back(sf::Vector2f(Config::world_width, border_y));
                        }
                    }
                    positions.push_back(proj);
                }
                return positions;
            }
            break;
    }
    return footprint;
}

float BasicInstrument::getResourceRate(std::string rname) const
{
    if(rname == "energy") {
        return m_energy_rate;
    // } else if(rname == "storage") {
        // return m_storage_rate;
    } else {
        return 0.f;
    }
}

std::map<std::string, float> BasicInstrument::getResourceRates(void) const
{
    std::map<std::string, float> rmap;
    rmap["energy"] = m_energy_rate;
    // rmap["storage"] = m_storage_rate;
    return rmap;
}


void BasicInstrument::enable(void)
{
    m_enabled = true;
}

void BasicInstrument::disable(void)
{
    m_enabled = false;
}
