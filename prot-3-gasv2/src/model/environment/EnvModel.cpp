/***********************************************************************************************//**
 *  Estimated state of the environment that the Agents capture and get payoff from.
 *  @class      EnvModel
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-10
 *  @version    0.2
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "EnvModel.hpp"
#include "Agent.hpp"

CREATE_LOGGER(EnvModel)

EnvModel::EnvModel(Agent* aptr, unsigned int mw, unsigned int mh)
    : m_agent(aptr)
    , m_model_w(mw)
    , m_model_h(mh)
    , m_world_w(World::getWidth())
    , m_world_h(World::getHeight())
    , m_view_payoff(nullptr)
{
    if(m_model_w == 0 || m_model_h == 0) {
        Log::warn << "Model can't have 0-length dimensions (" << m_model_w << ", " << m_model_h << ").\n";
        m_ratio_w = 1.f;
        m_ratio_h = 1.f;
        m_model_w = m_world_w;
        m_model_h = m_world_h;
    } else if(m_model_w > m_world_w || m_model_h > m_world_h) {
        Log::warn << "Model can't be bigger than the world (" << m_model_w << ", " << m_model_h << ").\n";
        m_ratio_w = 1.f;
        m_ratio_h = 1.f;
        m_model_w = m_world_w;
        m_model_h = m_world_h;
    } else {
        m_ratio_w = (float)m_world_w / (float)m_model_w;
        m_ratio_h = (float)m_world_h / (float)m_model_h;
    }

    m_cells.reserve(m_model_w);
    for(unsigned int i = 0; i < m_model_w; i++) {
        std::vector<EnvCell> row;
        row.reserve(m_model_h);
        for(unsigned int j = 0; j < m_model_h; j++) {
            EnvCell c(i, j);
            c.pushPayoffFunc(PayoffFunctions::f_revisit_time_backwards);
            row.push_back(c);
        }
        m_cells.push_back(row);
    }
}

void EnvModel::updateView(void)
{
}

void EnvModel::clearView(void)
{
}

void EnvModel::enableViewUnits(std::vector<sf::Vector2i> us)
{
}

const GridView& EnvModel::getView(void) const
{
    if(m_view_payoff == nullptr) {
        Log::err << "Environment view for agent " << m_agent->getId() << " has not been initialized but has just been requested.\n";
        throw std::runtime_error("Uninitialized environment view requested");
    }
    return *m_view_payoff;
}


std::vector<sf::Vector2i> EnvModel::getWorldCells(EnvCell model_cell) const
{
    return getWorldCells(sf::Vector2i(model_cell.x, model_cell.y));
}

std::vector<sf::Vector2i> EnvModel::getWorldCells(unsigned int x, unsigned int y) const
{
    return getWorldCells(sf::Vector2i(x, y));
}

std::vector<sf::Vector2i> EnvModel::getWorldCells(sf::Vector2i model_cell) const
{
    std::vector<sf::Vector2i> retval;
    int length_hor = m_ratio_w;
    int length_ver = m_ratio_h;
    int start_x = ((float)model_cell.x * m_ratio_w);
    int start_y = ((float)model_cell.y * m_ratio_h);
    for(int xx = 0; xx < length_hor; xx++) {
        for(int yy = 0; yy < length_ver; yy++) {
            retval.push_back(sf::Vector2i(start_x + xx, start_y + yy));
        }
    }
    return retval;
}

std::vector<sf::Vector2i> EnvModel::getWorldCells(std::vector<sf::Vector2i> model_cells) const
{
    std::unordered_set<sf::Vector2i, Vector2iHash> total_world_cells;
    for(const auto& model_cell : model_cells) {
        auto world_cells = getWorldCells(model_cell);
        for(auto wc : world_cells) {
            total_world_cells.insert(wc);
        }
    }
    return std::vector<sf::Vector2i>(total_world_cells.begin(), total_world_cells.end());
}
