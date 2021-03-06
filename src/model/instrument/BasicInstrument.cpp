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
    : BasicInstrument(Random::getUf(Config::agent_aperture_min, Config::agent_aperture_max), -1.f)
{ }

BasicInstrument::BasicInstrument(float aperture, float max_h)
    : m_swath(-1.f)
    , m_energy_rate(Random::getUf(Config::instrument_energy_min, Config::instrument_energy_max))
    , m_storage_rate(Random::getUf(Config::instrument_storage_min, Config::instrument_storage_max))
    , m_enabled(false)
{
    setAperture(aperture, max_h);
}

void BasicInstrument::setAperture(float ap, float max_h)
{
    float max_ap = findMaxAperture(max_h);
    if(ap > max_ap && Config::motion_model == AgentMotionType::ORBITAL) {
        m_aperture = max_ap;
        Log::warn << "Instrument aperture is too wide: " << ap
            << " (> " << max_ap << "º). Ignored and set to " << m_aperture << "º.\n";
    } else {
        m_aperture = ap;
    }
}

void BasicInstrument::setPosition(sf::Vector3f p)
{
    m_position = p;
    if(m_swath == -1.f && Config::motion_model == AgentMotionType::ORBITAL) {
        m_swath = getSwath(p, m_aperture);
        Log::dbg << "Instrument has a true swath of " << m_swath / 1e3 << " km. (aperture is " << m_aperture << "º). ";
        Log::dbg << "Max. slant range (at " << m_aperture / 2.f << "º) is: " << getSlantRangeAt(m_aperture / 2.f, m_position) / 1e3 << " km.\n";
    } else if(m_swath == -1.f && Config::motion_model != AgentMotionType::ORBITAL) {
        m_swath = m_aperture;
        Log::dbg << "Instrument has a swath/aperture of " << m_swath << " (2-d motion model).\n";
    }
}

float BasicInstrument::findMaxAperture(float h)
{
    if(h >= 0.f) {
        float ap_half = MathUtils::radToDeg(std::asin(Config::earth_radius / h));
        return ap_half * 2.f;
    } else {
        return Config::agent_aperture_max;
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
    double r,
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
    Utils::safeXY(ox, oy, lut);
    auto o_ecef = lut.at(ox).at(oy);
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
        Utils::safeXY(xit, yit, lut);
        auto s_ecef = lut.at(xit).at(yit);
        /*  ===== Original method: =================================================================
         *  Computes distance from p to s and compares with slant range. This methods yielded errors
         *  for cases with small swath/apertures (or the errors were not detected in the other
         *  cases). Distance from o to p resulted in longer distance than the slant range, which is
         *  impossible. This method has been disabled and left below for reference.
         **/
        #if 0
        float dist = MathUtils::norm(p_ecef - s_ecef);
        if(dist <= r) {
            f(xit, yit);
            at_r = true;
        }
        #endif

        /*  ===== New method: ======================================================================
         *  Computes the arc between points s and o and compares it with the current instrument
         *  swath. This method relies upon the great-circle distance method, and uses constant Earth
         *  radius 6371 km (i.e. approximates Earth to a sphere).
         **/
        float dist = MathUtils::arc(MathUtils::makeUnitary(o_ecef), MathUtils::makeUnitary(s_ecef)) * Config::earth_radius;
        if(dist <= r) {
            f(xit, yit);
            at_r = true;
        }
        if(verbose) {
            Log::dbg << "Point (" << xit << ", " << yit << ", " << (world_cells ? "[W]" : "[M]") << ") is at distance " << dist << " (> " << r << ").\n";
            Log::dbg << "  p = [" << p_ecef.x << ", " << p_ecef.y << ", " << p_ecef.z << "].\n";
            Log::dbg << "  s = [" << s_ecef.x << ", " << s_ecef.y << ", " << s_ecef.z << "].\n";
        }
    };
    ox %= lut.size();
    oy %= lut.at(0).size();
    // /*
    check_cell(ox, oy);
    if(!at_r) {
        check_cell(ox, oy, true);
        auto o_ecef = lut.at(ox).at(oy);
        auto o_eci = CoordinateSystemUtils::fromECEFToECI(o_ecef, t);
        // Log::warn << "Position in ECI  = (" << p.x << ", " << p.y << ", " << p.z << ").\n";
        // Log::warn << "Origin   in ECI  = (" << o_eci.x << ", " << o_eci.y << ", " << o_eci.z << ").\n";
        // Log::warn << "Position in ECEF = [" << p_ecef.x << ", " << p_ecef.y << ", " << p_ecef.z << "].\n";
        // Log::warn << "Origin   in ECEF = [" << o_ecef.x << ", " << o_ecef.y << ", " << o_ecef.z << "].\n";
        Log::warn << "Distance in ECEF: " << MathUtils::norm(p - o_eci) << ".\n";
        Log::warn << "Distance in ECI:  "  << MathUtils::norm(p_ecef - o_ecef) << ".\n";
    }
    // if(ox == 1714 && oy == 256) std::exit(1);
    // */

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
    double dist, sf::Vector3f position,
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
        Log::err << "Computing visible cells in motion type different than ORBITAL is deprecated.\n";
        throw std::runtime_error("Computing visible cells in motion type different than ORBITAL is deprecated.");
    } else {
        if(t <= -1.0) {
            t = VirtualTime::now();
        }
        auto proj = AgentMotion::getProjection2D(position, t);
        if(world_cells) {
            ox = proj.x;
            oy = proj.y;
        } else {
            ox = (int)std::floor(proj.x / m_env_info.rw) % m_env_info.mw;
            oy = (int)std::floor(proj.y / m_env_info.rh) % m_env_info.mh;
        }
        applyToDistance3D(ox, oy, position, t, dist, world_cells, f, lut);
    }
    if(cells.size() == 0) {
        #if 0
        Log::err << "Could not find visible (" << (world_cells ? "world" : "model") << ") cells at distance " << dist << ".\n";
        Log::err << "  Will provide one (i.e. the projected position) and try to continue.\n";

        /* DEBUG details: */
        auto proj = AgentMotion::getProjection2D(position, t);
        Log::err << "Origin calculation details: \n";
        Log::err << "  Position (ECI) = (" << position.x << ", " << position.y << ", " << position.z << "), has given the following:\n";
        Log::err << "  Projection     = (" << proj.x << ", " << proj.y << ")\n";
        sf::Vector3f ecef_coord = CoordinateSystemUtils::fromECIToECEF(position, t);
        sf::Vector3f geo_coord  = CoordinateSystemUtils::fromECEFToGeographic(ecef_coord); /* lat(x), lng(y), h(z). */
        Log::err << "This operation has been computed with two sequential transformations: \n";
        Log::err << "  ECI -> ECEF    = (" << ecef_coord.x << ", " << ecef_coord.y << ", " << ecef_coord.z << "), and then:\n";
        Log::err << "  ECEF -> Geo.   = {\n";
        Log::err << "    lng(y): " << geo_coord.y << "   -->   X: "
            << ( World::getWidth() * ( geo_coord.y) / 360.f) + (World::getWidth() / 2.f) << ".\n";
        Log::err << "    lat(x): " << geo_coord.x << "   -->   Y: "
            << (World::getHeight() * (-geo_coord.x) / 180.f) + (World::getHeight() / 2.f) << ".\n";
        Log::err << "  }\n";
        Log::err << "This geographic values in World coordinates are then converted to Model coordinates:\n";
        Log::err << "  std::floor(proj.x / m_env_info.rw) = std::floor("
            << proj.x << " / " << m_env_info.rw << ") = " << (int)std::floor(proj.x / m_env_info.rw) << "\n";
        Log::err << "  std::floor(proj.x / m_env_info.rw) = std::floor("
            << proj.y << " / " << m_env_info.rh << ") = " << (int)std::floor(proj.y / m_env_info.rh) << ". And then:\n";
        Log::err << "    modulo(" << m_env_info.mw << ")  = " << ox << ".\n";
        Log::err << "    modulo(" << m_env_info.mh << ")  = " << oy << ".\n";
        #endif
        cells.push_back(sf::Vector2i(ox, oy));
    }
    return cells;
}

std::vector<sf::Vector2i> BasicInstrument::getVisibleCells(double dist, sf::Vector2f position, bool world_cells) const
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
        return getVisibleCells(lut, getSwath(m_position, m_aperture) / 2.f, m_position, world_cells);
    } else {
        return getVisibleCells(lut, m_swath, m_position, world_cells);
    }
}

std::vector<sf::Vector2i> BasicInstrument::getVisibleCellsFromTo(const std::vector<std::vector<sf::Vector3f> >& lut,
    double ap, sf::Vector3f p0, sf::Vector3f p1, double t0, double t1, bool world_cells
) const
{
    if(Config::interpos < 2) {
        return getVisibleCells(lut, getSwath(p1, ap) / 2.f, p1, world_cells, t1);
    } else {
        sf::Vector3f pi;
        auto dvec = (p1 - p0) / ((float)Config::interpos - 1.f);
        std::vector<sf::Vector3f> posit_interp;
        std::vector<float> swath_interp;
        posit_interp.push_back(p0);
        swath_interp.push_back(getSwath(p0, ap));
        pi = p0;
        int pcount = 1;
        while(pcount < (Config::interpos - 1)) {
            pi += dvec;
            pcount++;
            posit_interp.push_back(pi);
            swath_interp.push_back(getSwath(pi, ap));
        }
        posit_interp.push_back(p1);
        swath_interp.push_back(getSwath(p1, ap));

        std::unordered_set<sf::Vector2i, Vector2iHash> retset;
        double ti = t0;
        double dt = (t1 - t0) / (double)(posit_interp.size() - 1);
        for(unsigned int j = 0; j < posit_interp.size(); j++) {
            auto fpj = getVisibleCells(lut, swath_interp[j] / 2.f, posit_interp[j], world_cells, ti);
            for(auto& fp_cell : fpj) {
                retset.insert(fp_cell);
            }
            ti += dt;
        }
        return std::vector<sf::Vector2i>(retset.begin(), retset.end());
    }
}

float BasicInstrument::getSlantRangeAt(long double deg, sf::Vector3f p) const
{
    /*  NOTE:
     *  Precise computation of slant angles should consider the actual radius of the Earth at the
     *  point of observation. Since we are not interested in extremely precise figures nor want to
     *  perform geographic corrections at this point, we'll assume that the radius of the Earth is
     *  constant and equal to its equatorial value (i.e. semi-major axis of WGS-86 ellipsoid).
     **/
    long double ang_rad = MathUtils::degToRad(std::fmod((long double)deg, 180.0L));
    long double h = MathUtils::norm(sf::Vector3<long double>(p.x, p.y, p.z));
    long double lambda = Config::pi - std::asin((h / Config::earth_wgs84_a) * std::sin(ang_rad));
    long double alpha  = Config::pi - lambda - ang_rad;
    long double sr = Config::earth_wgs84_a * std::sin(alpha) / std::sin(ang_rad);
    if(std::isnan(sr) || sr <= 1.0) {
        Log::err << "Computing slant range gave \'" << sr << "\' for instrument at " << deg << "º\n";
        Log::err << "  h = " << h << " meters.\n";
        Log::err << "  (h/R)·sin(ẟ) = " << (h / Config::earth_wgs84_a) * std::sin(ang_rad) << ".\n";
        Log::err << "  ẟ = " << deg << "º = " << ang_rad << " rad.\n";
        Log::err << "  λ = " << MathUtils::radToDeg(lambda) << "º = " << lambda << " rad.\n";
        Log::err << "  α = " << MathUtils::radToDeg(alpha) << "º = " << alpha << " rad.\n";
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
    switch(Config::motion_model) {
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
                float ang_rad = MathUtils::degToRad(m_aperture / 2.f);
                float h = MathUtils::norm(m_position);
                float lambda = Config::pi - std::asin((h / Config::earth_wgs84_a) * std::sin(ang_rad));
                float alpha  = Config::pi - lambda - ang_rad;
                float c_len  = Config::earth_wgs84_a * std::cos(alpha);

                float r = getSlantRangeAt(m_aperture / 2.f, m_position);
                float footprint_radius = r * std::sin(ang_rad);

                sf::Vector3f u = MathUtils::makeUnitary(m_position);
                sf::Vector3f c = u * c_len;

                /*  Obtain two perpendicular vectors to `u`: `a` and `b`.
                 *  - `a` is obtained as a non-null linear combination of `u`.
                 *  - `b` is obtained as the cross-product of `u` and `a`.
                 **/
                sf::Vector3f a = MathUtils::makeUnitary(sf::Vector3f(u.z, u.z, (-u.x - u.y)));
                if(a == sf::Vector3f(0.f, 0.f, 0.f)) {
                    a = sf::Vector3f((-u.y - u.z), u.x, u.x);
                }

                a = MathUtils::makeUnitary(a);

                sf::Vector3f b;
                b.x = u.y * a.z - u.z * a.y;
                b.y = -(u.x * a.z - u.z * a.x);
                b.z = u.x * a.y - u.y * a.x;
                b = MathUtils::makeUnitary(b);

                auto p_proj = AgentMotion::getProjection2D(m_position, VirtualTime::now());

                /* Compute number of points based on "latitude" (Y-axis value) */
                float lat = p_proj.y;
                if(lat >= (Config::world_height - m_fp_lat_threshold)) {
                    lat = -(lat - Config::world_height);
                }
                float points_inc = (m_max_fp_points - m_min_fp_points) / m_fp_lat_threshold;
                if (lat <= m_fp_lat_threshold) {
                    n_points = m_min_fp_points + points_inc * lat;
                } else {
                    n_points = m_min_fp_points;
                }

                sf::Vector2f top_left  = sf::Vector2f(-10.f, -10.f);
                sf::Vector2f top_right = sf::Vector2f(Config::world_width + 10.f, -10.f);

                /*  Get the boundaries of longitude quadrants:
                 *  - 1st quadrant: [0, xq1),
                 *  - 2nd quadrant: [xq1, xq2),
                 *  - 3rd quadrant: [xq2, xq3),
                 *  - 4th quadrant: [xq3, xq4) = [xq3, Config::world_width) = [xq3, 0).
                 *  Values xq2 and xq4 are actually not used and hence will not be calculated.
                 **/
                float xq1 = Config::world_width / 4.f;
                float xq3 = 3.f * Config::world_width / 4.f;

                bool first_point = true;
                float da = 2.f * Config::pi / (float)n_points;
                for(int i = 0; i <= n_points; i++) {
                    float footprint_x = c.x + footprint_radius * std::cos(da * (float)i) * a.x + footprint_radius * std::sin(da * (float)i) * b.x;
                    float footprint_y = c.y + footprint_radius * std::cos(da * (float)i) * a.y + footprint_radius * std::sin(da * (float)i) * b.y;
                    float footprint_z = c.z + footprint_radius * std::cos(da * (float)i) * a.z + footprint_radius * std::sin(da * (float)i) * b.z;

                    sf::Vector3f fp_pos  = sf::Vector3f(footprint_x, footprint_y, footprint_z);
                    sf::Vector2f fp_proj = AgentMotion::getProjection2D(fp_pos, VirtualTime::now());

                    sf::Vector2f fp_prev;
                    if(footprint.size() > 0) {
                        fp_prev = footprint.back();
                    } else {
                        fp_prev = fp_proj;
                    }

                    bool is_split;
                    if(fp_proj.x > xq3 && fp_prev.x < xq1) {
                        /* This point is on the 4th quad. and the previous one was on the 1st quad. */
                        is_split = true;
                    } else if(fp_proj.x < xq1 && fp_prev.x > xq3) {
                        /* This point is on the 1st quad. and the previous one was on the 4th quad. */
                        is_split = true;
                    } else {
                        is_split = false;
                    }

                    if(!first_point) {
                        if(is_split) {
                            if(fp_prev.x > fp_proj.x) {
                                footprint.push_back({ (fp_proj.x + Config::world_width), fp_proj.y });
                                footprint.push_back(top_right);
                                footprint.push_back(top_left);
                                footprint.push_back({ (fp_prev.x - Config::world_width), fp_prev.y });
                                footprint.push_back(fp_proj);
                            } else {
                                footprint.push_back({ (fp_proj.x - Config::world_width), fp_proj.y });
                                footprint.push_back(top_left);
                                footprint.push_back(top_right);
                                footprint.push_back({ (fp_prev.x + Config::world_width), fp_prev.y });
                                footprint.push_back(fp_proj);
                            }
                        } else {
                            footprint.push_back(fp_proj);
                        }
                    } else {
                        footprint.push_back(fp_proj);
                    }
                    first_point = false;
                }
                break;
            }
    }
    return footprint;
}

double BasicInstrument::getResourceRate(std::string rname) const
{
    if(rname == "energy") {
        return m_energy_rate;
    // } else if(rname == "storage") {
        // return m_storage_rate;
    } else {
        return 0.f;
    }
}

std::map<std::string, double> BasicInstrument::getResourceRates(void) const
{
    std::map<std::string, double> rmap;
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
