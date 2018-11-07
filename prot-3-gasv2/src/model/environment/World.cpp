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
    : m_self_view(m_width, m_height, 1.f, 1.f)
{
    std::time_t t = std::time(nullptr);
    char str[100];
    std::strftime(str, sizeof(str), "%Y_%m_%d_%H%M%S", std::localtime(&t));
    std::string time_str(str);
    m_report_file.open(Config::root_path + "data/" + time_str + ".csv");
    m_report_file << "Time,"
        << "Mean_best_coverage,"
        << "Mean_actual_coverage,"
        << "Mean_best_revisit_time,"
        << "Max_best_revisit_time,"
        << "Min_best_revisit_time,"
        << "Mean_actual_revisit_time,"
        << "Max_actual_revisit_time,"
        << "Min_actual_revisit_time,"
        << "Mean_overlapping,"
        << "Worst_overlapping,"
        << "Max_overlapping\n";

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
            layers[(int)Layer::OVERLAPPING_WORST].value = 0.f;
            layers[(int)Layer::OVERLAPPING_ACTUAL].value = 0.f;
            row.push_back(layers);
        }
        m_cells.push_back(row);
    }
}

World::~World(void)
{
    m_report_file.close();
}

void World::report(void)
{
    m_report_file << VirtualTime::now() << ","
        << m_mean_best_coverage << ","
        << m_mean_actual_coverage << ","
        << m_mean_best_revisit_time << ","
        << m_max_best_revisit_time << ","
        << m_min_best_revisit_time << ","
        << m_mean_actual_revisit_time << ","
        << m_max_actual_revisit_time << ","
        << m_min_actual_revisit_time << ","
        << m_mean_overlapping << ","
        << m_worst_overlapping << ","
        << m_max_overlapping << "\n";
    m_report_file << std::flush;
}

void World::display(Layer l)
{
    float norm_val, cell_val;
    float mean_val = 0.f;
    float max_val  = 0.f;
    float min_val  = 1.f;
    // #pragma omp parallel for default(shared) reduction(+:mean_val) reduction(max:max_val) reduction(min:min_val)
    for(unsigned int i = 0; i < m_width; i++) {
        for(unsigned int j = 0; j < m_height; j++) {
            cell_val = m_cells[i][j][(int)l].value;
            if(l == Layer::REVISIT_TIME_BEST || l == Layer::REVISIT_TIME_ACTUAL) {
                norm_val = 1.f - (cell_val / Config::max_revisit_time);
            } else {
                norm_val = cell_val;
            }
            m_self_view.setValue(i, j, norm_val);
            mean_val += norm_val;
            max_val = (max_val > norm_val ? max_val : norm_val);
            min_val = (min_val < norm_val ? min_val : norm_val);
        }
    }
    mean_val /= (float)(m_width * m_height);

    switch(l) {
        case Layer::COVERAGE_BEST:
            m_mean_best_coverage = mean_val;
            break;
        case Layer::COVERAGE_ACTUAL:
            m_mean_actual_coverage = mean_val;
            break;
        case Layer::REVISIT_TIME_BEST:
            m_mean_best_revisit_time = mean_val;
            m_max_best_revisit_time  = max_val;
            m_min_best_revisit_time  = min_val;
            break;
        case Layer::REVISIT_TIME_ACTUAL:
            m_mean_actual_revisit_time = mean_val;
            m_max_actual_revisit_time  = max_val;
            m_min_actual_revisit_time  = min_val;
            break;
        case Layer::OVERLAPPING_WORST:
            m_worst_overlapping  = max_val;
            break;
        case Layer::OVERLAPPING_ACTUAL:
            m_mean_overlapping = mean_val;
            m_max_overlapping  = max_val;
            break;
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
            m_cells[xx][yy][(int)Layer::OVERLAPPING_WORST].value = 0.f;
            m_cells[xx][yy][(int)Layer::OVERLAPPING_ACTUAL].value = 0.f;
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
            updateLayer(Layer::OVERLAPPING_WORST, c.x, c.y, true);
            updateLayer(Layer::OVERLAPPING_ACTUAL, c.x, c.y, capturing);
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
        case Layer::OVERLAPPING_WORST:
        case Layer::OVERLAPPING_ACTUAL:
            if(active) {
                cell.value += 1.f;
            }
            break;
        default:
            Log::err << "Unrecognized layer " << (unsigned int)l;
            break;
    }
}
