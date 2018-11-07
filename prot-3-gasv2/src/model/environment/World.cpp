/***********************************************************************************************//**
 *  The world in which Agents actually live.
 *  @class      World
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-19
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "World.hpp"
#include "Agent.hpp"

CREATE_LOGGER(World)

unsigned int World::m_width = Config::world_width;
unsigned int World::m_height = Config::world_height;

World::World(void)
    : m_self_view(m_width, m_height, 1.f, 1.f, Config::color_gradient_rainbow.getColorAt(0.f))
{
    m_cells.reserve(m_width);
    for(unsigned int i = 0; i < m_width; i++) {
        std::vector<std::vector<WorldCell> > row;
        row.reserve(m_height);
        for(unsigned int j = 0; j < m_height; j++) {
            std::vector<WorldCell> layers(n_layers, { 0.f });
            /* Initialize values for each layer: */
            layers[(int)Layer::COVERAGE_BEST].value = 0.f;
            layers[(int)Layer::COVERAGE_ACTUAL].value = 0.f;
            layers[(int)Layer::REVISIT_TIME_BEST].value = Config::max_revisit_time;
            layers[(int)Layer::REVISIT_TIME_ACTUAL].value = Config::max_revisit_time;
            row.push_back(layers);
        }
        m_cells.push_back(row);
    }
}

void World::display(Layer l)
{
    float norm_val, cell_val;
    #pragma omp parallel for
    for(unsigned int i = 0; i < m_width; i++) {
        for(unsigned int j = 0; j < m_height; j++) {
            cell_val = m_cells[i][j][(int)l].value;
            if(l == Layer::REVISIT_TIME_BEST || l == Layer::REVISIT_TIME_ACTUAL) {
                norm_val = 1.f - (cell_val / Config::max_revisit_time);
            } else {
                norm_val = cell_val;
            }
            m_self_view.setValue(i, j, norm_val);
        }
    }
}

void World::addAgent(std::shared_ptr<Agent> aptr)
{
    m_agents.push_back(aptr);
}

void World::addAgent(std::vector<std::shared_ptr<Agent> > aptrs)
{
    for(auto& a : aptrs) {
        m_agents.push_back(a);
    }
}

void World::step(void)
{
    #pragma omp parallel for
    for(unsigned int xx = 0; xx < m_width; xx++) {
        for(unsigned int yy = 0; yy < m_height; yy++) {
            updateAllLayers(xx, yy, false);
        }
    }
    for(auto& a : m_agents) {
        auto cells = a->getWorldFootprint();
        bool capturing = a->isCapturing();
        for(auto& c : cells) {
            updateLayer(Layer::COVERAGE_BEST, c.x, c.y, true);
            updateLayer(Layer::COVERAGE_ACTUAL, c.x, c.y, capturing);
            updateLayer(Layer::REVISIT_TIME_BEST, c.x, c.y, true);
            updateLayer(Layer::REVISIT_TIME_ACTUAL, c.x, c.y, capturing);
        }
    }
}

void World::updateAllLayers(int x, int y, bool active)
{
    for(unsigned int l = 0; l < n_layers; l++) {
        updateLayer(static_cast<Layer>(l), x, y, active);
    }
}

void World::updateLayer(Layer l, int x, int y, bool active)
{
    auto& cell = m_cells[x][y][(int)l];
    switch(l) {
        case Layer::COVERAGE_BEST:
        case Layer::COVERAGE_ACTUAL:
            if(active) {
                cell.value = 1.f;
            } else {
                cell.value = 0.f;
            }
            break;
        case Layer::REVISIT_TIME_BEST:
        case Layer::REVISIT_TIME_ACTUAL:
            if(active) {
                cell.value = 0.f;
            } else {
                if(cell.value < Config::max_revisit_time) {
                    cell.value += Config::time_step;
                }
                if(cell.value > Config::max_revisit_time) {
                    cell.value = Config::max_revisit_time;
                }
            }
            break;
        default:
            Log::err << "Unrecognized layer " << (unsigned int)l;
            break;
    }
}
