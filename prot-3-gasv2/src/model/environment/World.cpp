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
std::vector<std::vector<sf::Vector3f> > World::m_world_positions;

World::World(void)
    : ReportGenerator("world_metrics.csv")
    , m_self_view(m_width, m_height, 1.f, 1.f, Config::color_gradient_rainbow.getColorAt(0.f))
{
    addReportColumn("Mean_best_coverage");          /* 0 */
    addReportColumn("Mean_actual_coverage");        /* 1 */
    addReportColumn("Mean_best_revisit_time");      /* 2 */
    addReportColumn("Max_best_revisit_time");       /* 3 */
    addReportColumn("Min_best_revisit_time");       /* 4 */
    addReportColumn("Mean_actual_revisit_time");    /* 5 */
    addReportColumn("Max_actual_revisit_time");     /* 6 */
    addReportColumn("Min_actual_revisit_time");     /* 7 */
    addReportColumn("Mean_overlapping");            /* 8 */
    addReportColumn("Worst_overlapping");           /* 9 */
    addReportColumn("Max_overlapping");             /* 10 */
    enableReport();

    m_cells.reserve(m_width);
    for(unsigned int i = 0; i < m_width; i++) {
        std::vector<std::vector<WorldCell> > column;
        column.reserve(m_height);
        for(unsigned int j = 0; j < m_height; j++) {
            std::vector<WorldCell> layers(n_layers, { 0.f });
            /* Initialize values for each layer: */
            layers[(int)Layer::COVERAGE_BEST].value = 0.f;
            layers[(int)Layer::COVERAGE_ACTUAL].value = 0.f;
            layers[(int)Layer::REVISIT_TIME_BEST].value = Config::max_revisit_time;
            layers[(int)Layer::REVISIT_TIME_ACTUAL].value = Config::max_revisit_time;
            layers[(int)Layer::OVERLAPPING_WORST].value = 0.f;
            layers[(int)Layer::OVERLAPPING_ACTUAL].value = 0.f;
            column.push_back(layers);
        }
        m_cells.push_back(column);
    }
    if(m_world_positions.size() == 0 && Config::motion_model == AgentMotionType::ORBITAL) {
        /* Build the positions LUT: */
        float lat, lng;
        m_world_positions.reserve(m_width);
        for(unsigned int i = 0; i < m_width; i++) {
            std::vector<sf::Vector3f> column_lut;
            column_lut.reserve(m_height);
            for(unsigned int j = 0; j < m_height; j++) {
                lng =   (360.f * i / World::getWidth()) - 180.f;
                lat = -((180.f * j / World::getHeight()) - 90.f);
                auto position_ecef = CoordinateSystemUtils::fromGeographicToECEF(sf::Vector3f(lat, lng, 0.f));
                column_lut.push_back(position_ecef);
            }
            m_world_positions.push_back(column_lut);
        }
        Log::dbg << "Completed the pre-computation of coordinates in the ECEF frame for every world cell.\n";
    }
}

void World::display(Layer l)
{
    float cell_val, norm_val;
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

void World::computeMetrics(void)
{
    #pragma omp parallel for
    for(unsigned int l = 0; l < n_layers; l++) {
        float norm_val, cell_val;
        float mean_val = 0.f;
        float max_val  = 0.f;
        float min_val  = 1.f;
        for(unsigned int i = 0; i < m_width; i++) {
            for(unsigned int j = 0; j < m_height; j++) {
                cell_val = m_cells[i][j][l].value;
                if(static_cast<Layer>(l) == Layer::REVISIT_TIME_BEST || static_cast<Layer>(l) == Layer::REVISIT_TIME_ACTUAL) {
                    norm_val = 1.f - (cell_val / Config::max_revisit_time);
                } else {
                    norm_val = cell_val;
                }
                mean_val += norm_val;
                max_val = (max_val > norm_val ? max_val : norm_val);
                min_val = (min_val < norm_val ? min_val : norm_val);
            }
        }
        mean_val /= (float)(m_width * m_height);

        switch(static_cast<Layer>(l)) {
            case Layer::COVERAGE_BEST:
                setReportColumnValue(0, mean_val);  /* Mean_best_coverage. */
                break;
            case Layer::COVERAGE_ACTUAL:
                setReportColumnValue(1, mean_val);  /* Mean_actual_coverage. */
                break;
            case Layer::REVISIT_TIME_BEST:
                setReportColumnValue(2, mean_val);  /* Mean_best_revisit_time. */
                setReportColumnValue(3, max_val);   /* Max_best_revisit_time. */
                setReportColumnValue(4, min_val);   /* Min_best_revisit_time. */
                break;
            case Layer::REVISIT_TIME_ACTUAL:
                setReportColumnValue(5, mean_val);  /* Mean_actual_revisit_time. */
                setReportColumnValue(6, max_val);   /* Max_actual_revisit_time. */
                setReportColumnValue(7, min_val);   /* Min_actual_revisit_time. */
                break;
            case Layer::OVERLAPPING_WORST:
                setReportColumnValue(9, max_val);   /* Worst_overlapping. */
                break;
            case Layer::OVERLAPPING_ACTUAL:
                setReportColumnValue(8, mean_val);  /* Mean_overlapping. */
                setReportColumnValue(10, max_val);  /* Max_overlapping. */
                break;
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
            m_cells[xx][yy][(int)Layer::OVERLAPPING_WORST].value = 0.f;
            m_cells[xx][yy][(int)Layer::OVERLAPPING_ACTUAL].value = 0.f;
            updateAllLayers(xx, yy, false);
        }
    }
    for(auto& a : m_agents) {
        auto cells = a->getWorldFootprint(m_world_positions);
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
