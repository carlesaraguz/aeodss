/***********************************************************************************************//**
 *  Conical, nadir-pointing instrument.
 *  @class      BasicInstrument
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-10
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "BasicInstrument.hpp"

CREATE_LOGGER(BasicInstrument)

BasicInstrument::BasicInstrument(void)
    : BasicInstrument(Random::getUf(Config::agent_swath_min, Config::agent_swath_max))
{ }

BasicInstrument::BasicInstrument(float world_swath)
    : m_swath(world_swath)
    , m_energy_rate(Random::getUf(Config::instrument_energy_min, Config::instrument_energy_max))
    , m_storage_rate(Random::getUf(Config::instrument_storage_min, Config::instrument_storage_max))
    , m_enabled(false)
{ }

bool BasicInstrument::applyToDistance(
    unsigned int ox,
    unsigned int oy,
    float r,
    bool world_distance,
    std::function<void(unsigned int, unsigned int)> f_true,
    std::function<void(unsigned int, unsigned int)> f_false) const
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
                f_true(xx + ox, yy + oy);
                at_r = true;
                corner_count = 0;
            } else {
                f_false(xx + ox, yy + oy);
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

std::vector<sf::Vector2i> BasicInstrument::getVisibleCells(float swath, sf::Vector2f position) const
{
    int ox = std::round(position.x / m_env_info.rw);
    int oy = std::round(position.y / m_env_info.rh);
    if(ox > (int)m_env_info.mw || oy > (int)m_env_info.mh || ox < 0 || oy < 0) {
        Log::err << "Can't get visible cells for a point that is outside the model space O(" << ox << ", " << oy << ")\n";
        throw std::runtime_error("Can't compute visible cells if position is outside the environment space.");
    }
    float dist = swath / 2.f;
    std::vector<sf::Vector2i> cells;
    auto f = [&cells](unsigned int x, unsigned int y) {
        cells.push_back(sf::Vector2i(x, y));
    };
    if(!applyToDistance(ox, oy, dist, false, f)) {
        Log::err << "Failed to get visible (model) cells from different position: "
            << "(" << ox << ", "<< oy << ") <=> " << dist << ". Maximum number of operations reached.\n";
        throw std::runtime_error("Failed to get visible (model) cells from different position. Maximum number of operations reached.");
        return { };
    }
    return cells;
}

std::vector<sf::Vector2i> BasicInstrument::getVisibleCells(bool world_cells) const
{
    int ox, oy;
    float dist = m_swath / 2.f;
    if(world_cells) {
        ox = std::round(m_position.x);
        oy = std::round(m_position.y);
        if(ox > (int)Config::world_width || oy > (int)Config::world_height || ox < 0 || oy < 0) {
            Log::err << "Can't get visible cells for a point that is outside the world space O(" << ox << ", " << oy << ")\n";
            throw std::runtime_error("Can't compute visible cells if position is outside the environment space.");
        }
    } else {
        ox = std::round(m_position.x / m_env_info.rw);
        oy = std::round(m_position.y / m_env_info.rh);
        if(ox > (int)m_env_info.mw || oy > (int)m_env_info.mh || ox < 0 || oy < 0) {
            Log::err << "Can't get visible cells for a point that is outside the model space O(" << ox << ", " << oy << ")\n";
            throw std::runtime_error("Can't compute visible cells if position is outside the environment space.");
        }
    }

    std::vector<sf::Vector2i> cells;
    auto f = [&cells](unsigned int x, unsigned int y) {
        cells.push_back(sf::Vector2i(x, y));
    };
    if(!applyToDistance(ox, oy, dist, world_cells, f)) {
        Log::err << "Failed to get visible (" << (world_cells ? "world" : "model") << ") cells: "
            << "(" << ox << ", "<< oy << ") <=> " << dist << ". Maximum number of operations reached.\n";
        return { };
    }
    return cells;
}

std::vector<sf::Vector2f> BasicInstrument::getFootprint(void) const
{
    int N_POINTS = 16;
    switch(Config::motion_model){
        case AgentMotionType::LINEAR_BOUNCE:
            {
                float da = 2.f * Config::pi / (float)N_POINTS;
                std::vector<sf::Vector2f> footprint;
                for(int i = 0; i <= N_POINTS; i++) {
                    footprint.push_back(((m_swath / 2.f) * sf::Vector2f(std::cos(da * (float)i), std::sin(da * (float)i))));
                }
                return footprint;
            }
            break;

        case AgentMotionType::LINEAR_INFINITE:
            Log::err << "[ TODO ] Constructing AgentMotion with an unsupported motion model: LINEAR_INFINITE.\n";
            throw std::runtime_error("Constructing AgentMotion with unsupported motion model LINEAR_INFINITE.");
            break;

        case AgentMotionType::SINUSOIDAL:
            Log::err << "[ TODO ] Constructing AgentMotion with an unsupported motion model: SINUSOIDAL.\n";
            throw std::runtime_error("Constructing AgentMotion with unsupported motion model SINUSOIDAL.");
            break;

        case AgentMotionType::ORBITAL:
            {
                float R = Config::earth_radius;
                sf::Vector3f p = getPosition();

                float aperture = MathUtils::degToRad(getSwath());

                float h = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);

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

                sf::Vector3f a = sf::Vector3f();

                if(v == sf::Vector3f(1, 0, 0)) {
                    a = sf::Vector3f(0, 1, 0);
                } else {
                    a = sf::Vector3f(0, v.z, -v.y);
                }

                a = a / std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
                sf::Vector3f b = sf::Vector3f();

                b.x = v.y * a.z - v.z * a.y;
                b.y = v.x * a.z - v.z * a.x;
                b.z = v.x * a.y - v.y * a.x;

                b = b / std::sqrt(b.x * b.x + b.y * b.y + b.z * b.z);


                float step = 2 * Config::pi / N_POINTS;

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

                /* Core no passa abans d'aquí. */

                for(float th = 0; th < N_POINTS; th += step) {
                    float footprint_x = c.x + footprint_radius * std::cos(th) * a.x + footprint_radius * std::sin(th) * b.x;
                    float footprint_y = c.z + footprint_radius * std::cos(th) * a.y + footprint_radius * std::sin(th) * b.y;
                    float footprint_z = c.z + footprint_radius * std::cos(th) * a.z + footprint_radius * std::sin(th) * b.z;

                    sf::Vector3f fp_pos = sf::Vector3f(footprint_x, footprint_y, footprint_z);
                    sf::Vector2f proj   = AgentMotion::getProjection2D(fp_pos, VirtualTime::now());

                    sf::Vector2f prev = sf::Vector2f();

                    if (positions.size() > 0) {
                        prev = positions.back();
                    }

                    if(north_included) {

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

                    if(south_included) {

                        float border_y = std::abs(prev.y - proj.y) / 2;

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

                    bool is_splitted = false;

                    float x_distance = std::abs(proj.x - prev.x);

                    if(x_distance > Config::world_width / 2) {
                        is_splitted = true;
                    }

                    if(is_splitted){
                        /* Compute border point */
                        float border_y = std::abs(prev.y - proj.y) / 2;

                        if(prev.x > proj.x) {
                            positions.push_back(sf::Vector2f(Config::world_width, border_y));
                            positions.push_back(sf::Vector2f(Config::world_width, 0));
                            positions.push_back(sf::Vector2f(0, 0));
                            positions.push_back(sf::Vector2f(0, border_y));
                        } else {
                            positions.push_back(sf::Vector2f(0, border_y));
                            positions.push_back(sf::Vector2f(0, Config::world_height));
                            positions.push_back(sf::Vector2f(Config::world_width, Config::world_height));
                            positions.push_back(sf::Vector2f(Config::world_width, border_y));
                        }
                    }
                    if(proj.y != Config::world_height || proj.y != 0){
                        positions.push_back(proj);
                    }
                }

                return positions;
            }
            break;
        default:
            {
                /* Does nothing */
            }
            break;
    }

    std::vector<sf::Vector2f> vect;
    vect.push_back(sf::Vector2f());
    return vect;
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
