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
    float dist = swath / 2.f;
    std::vector<sf::Vector2i> cells;
    auto f = [&cells](unsigned int x, unsigned int y) {
        cells.push_back(sf::Vector2i(x, y));
    };
    if(!applyToDistance(ox, oy, dist, false, f)) {
        Log::err << "Failed to get visible (model) cells from different position. Maximum number of oterations reached\n";
        throw std::runtime_error("Error");
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
    } else {
        ox = std::round(m_position.x / m_env_info.rw);
        oy = std::round(m_position.y / m_env_info.rh);
    }
    std::vector<sf::Vector2i> cells;
    auto f = [&cells](unsigned int x, unsigned int y) {
        cells.push_back(sf::Vector2i(x, y));
    };
    if(!applyToDistance(ox, oy, dist, world_cells, f)) {
        Log::err << "Failed to get visible (" << (world_cells ? "world" : "model") << ") cells. Maximum number of oterations reached\n";
        return { };
    }
    return cells;
}

std::vector<sf::Vector2f> BasicInstrument::getFootprint(void) const
{
    int n = 16;
    float da = 2.f * Config::pi / (float)n;
    std::vector<sf::Vector2f> footprint;
    for(int i = 0; i <= n; i++) {
        footprint.push_back(((m_swath / 2.f) * sf::Vector2f(std::cos(da * (float)i), std::sin(da * (float)i))));
    }
    return footprint;
}

float BasicInstrument::getResourceRate(std::string rname) const
{
    if(rname == "energy") {
        return m_energy_rate;
    } else if(rname == "storage") {
        return m_storage_rate;
    } else {
        return 0.f;
    }
}

void BasicInstrument::enable(void)
{
    m_enabled = true;
}

void BasicInstrument::disable(void)
{
    m_enabled = false;
}
