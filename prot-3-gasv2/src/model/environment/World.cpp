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
    : ReportGenerator(std::string("world_metrics.csv"))
    , m_self_view(m_width, m_height, 1.f, 1.f, sf::Color(127, 127, 127))
{
    unsigned int wg = m_width / 4;
    unsigned int hg = m_height / 6;
    m_metrics_grids.push_back({ (0 * wg), (4 * wg), (0 * hg), (1 * hg) });
    m_metrics_grids.push_back({ (0 * wg), (2 * wg), (1 * hg), (2 * hg) });
    m_metrics_grids.push_back({ (2 * wg), (4 * wg), (1 * hg), (2 * hg) });
    m_metrics_grids.push_back({ (0 * wg), (1 * wg), (2 * hg), (3 * hg) });
    m_metrics_grids.push_back({ (1 * wg), (2 * wg), (2 * hg), (3 * hg) });
    m_metrics_grids.push_back({ (2 * wg), (3 * wg), (2 * hg), (3 * hg) });
    m_metrics_grids.push_back({ (3 * wg), (4 * wg), (2 * hg), (3 * hg) });
    m_metrics_grids.push_back({ (0 * wg), (1 * wg), (3 * hg), (4 * hg) });
    m_metrics_grids.push_back({ (1 * wg), (2 * wg), (3 * hg), (4 * hg) });
    m_metrics_grids.push_back({ (2 * wg), (3 * wg), (3 * hg), (4 * hg) });
    m_metrics_grids.push_back({ (3 * wg), (4 * wg), (3 * hg), (4 * hg) });
    m_metrics_grids.push_back({ (0 * wg), (2 * wg), (4 * hg), (5 * hg) });
    m_metrics_grids.push_back({ (2 * wg), (4 * wg), (4 * hg), (5 * hg) });
    m_metrics_grids.push_back({ (0 * wg), (4 * wg), (5 * hg), (5 * hg) });

    for(unsigned int i = 0; i < m_metrics_grids.size(); i++) {
        addReportColumn("utopia_avg" + std::to_string(i));      /* i x 0 */
        addReportColumn("utopia_max" + std::to_string(i));      /* i x 1 */
        addReportColumn("diff_avg" + std::to_string(i));        /* i x 2 */
        addReportColumn("diff_max" + std::to_string(i));        /* i x 3 */
        addReportColumn("actual_avg" + std::to_string(i));      /* i x 4 */
        addReportColumn("actual_max" + std::to_string(i));      /* i x 5 */
        addReportColumn("coverage" + std::to_string(i));        /* i x 6 */
        addReportColumn("coverage_avg" + std::to_string(i));    /* i x 7 */
    }
    addReportColumn("barcelona_utop");      /* 13 x 7 + 1 = 92 */
    addReportColumn("barcelona_actual");    /* 93 */
    addReportColumn("singapore_utop");      /* 94 */
    addReportColumn("singapore_actual");    /* 95 */
    addReportColumn("greenland_utop");      /* 96 */
    addReportColumn("greenland_actual");    /* 97 */
    addReportColumn("cape_town_utop");      /* 98 */
    addReportColumn("cape_town_actual");    /* 99 */
    enableReport();

    m_cells.reserve(m_width);
    for(unsigned int i = 0; i < m_width; i++) {
        std::vector<std::vector<WorldCell> > column;
        column.reserve(m_height);
        for(unsigned int j = 0; j < m_height; j++) {
            std::vector<WorldCell> layers(n_layers, { 0.f });
            /* Initialize values for each layer: */
            layers[(int)Layer::REVISIT_TIME_UTOPIA].value = -1.f;
            layers[(int)Layer::REVISIT_TIME_ACTUAL].value = Config::goal_target;
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
    #pragma omp parallel for
    for(unsigned int i = 0; i < m_width; i++) {
        for(unsigned int j = 0; j < m_height; j++) {
            float cell_val, norm_val;
            cell_val = m_cells[i][j][(int)l].value;
            if(cell_val >= 0.f) {
                norm_val = 1.f - (cell_val / (2.f * Config::goal_target));
                norm_val = std::max(norm_val, 0.f);
                m_self_view.setValue(i, j, norm_val);
            } else {
                m_self_view.setValue(i, j, -1.f);
            }
        }
    }
}

void World::computeMetrics(void)
{
    std::vector<float> avgs_utop(m_metrics_grids.size());
    std::vector<float> avgs_diff(m_metrics_grids.size());
    std::vector<float> avgs_curr(m_metrics_grids.size());
    std::vector<float> maxs_utop(m_metrics_grids.size());
    std::vector<float> maxs_diff(m_metrics_grids.size());
    std::vector<float> maxs_curr(m_metrics_grids.size());
    std::vector<float> rel_areas(m_metrics_grids.size());
    std::vector<float> avgs_unmet(m_metrics_grids.size());
    // #pragma omp parallel for shared(avgs_utop, avgs_diff, avgs_curr, maxs_utop, maxs_diff, maxs_curr, m_metrics_grids, m_cells)
    for(unsigned int q = 0; q < m_metrics_grids.size(); q++) {
        unsigned int x0, x1, y0, y1;
        // #pragma omp critical
        {
            x0 = m_metrics_grids[q].x0;
            x1 = m_metrics_grids[q].x1;
            y0 = m_metrics_grids[q].y0;
            y1 = m_metrics_grids[q].y1;
        }
        float utop_val = 0.f;
        float diff_val = 0.f;
        float curr_val = 0.f;
        float avg_val_utop = 0.f;
        float avg_val_diff = 0.f;
        float avg_val_curr = 0.f;
        float avg_val_unmet = 0.f;
        float max_val_utop = 0.f;
        float max_val_diff = 0.f;
        float max_val_curr = 0.f;
        int count_cells = 1;
        rel_areas[q] = 0.f;
        for(unsigned int i = x0; i < x1; i++) {
            for(unsigned int j = y0; j < y1; j++) {
                utop_val = m_cells[i][j][(int)Layer::REVISIT_TIME_UTOPIA].value;
                if(utop_val >= 0.f) {
                    curr_val = m_cells[i][j][(int)Layer::REVISIT_TIME_ACTUAL].value;
                    diff_val = curr_val - utop_val;
                    avg_val_utop += utop_val;
                    avg_val_curr += curr_val;
                    avg_val_diff += diff_val;
                    max_val_utop = (max_val_utop > utop_val ? max_val_utop : utop_val);
                    max_val_diff = (max_val_diff > diff_val ? max_val_diff : diff_val);
                    max_val_curr = (max_val_curr > curr_val ? max_val_curr : curr_val);
                    count_cells++;
                    if(curr_val > Config::goal_target) {
                        rel_areas[q] += 1.f;
                        avg_val_unmet += curr_val;
                    }
                }
            }
        }
        avg_val_utop /= (float)(count_cells);
        avg_val_diff /= (float)(count_cells);
        avg_val_curr /= (float)(count_cells);
        if(rel_areas[q] == 0.f) {
            avg_val_unmet = 0.f;            /* Leaves as 0. */
        } else {
            avg_val_unmet /= rel_areas[q];  /* Does the average. */
        }
        rel_areas[q] /= (float)(count_cells);
        // #pragma omp critical
        {
            avgs_utop[q] = avg_val_utop;
            avgs_diff[q] = avg_val_diff;
            avgs_curr[q] = avg_val_curr;
            avgs_unmet[q] = avg_val_unmet;
            maxs_utop[q] = max_val_utop;
            maxs_diff[q] = max_val_diff;
            maxs_curr[q] = max_val_curr;
        }
    }
    for(unsigned int q = 0; q < m_metrics_grids.size(); q++) {
        setReportColumnValue((8 * q) + 0, avgs_utop[q]);
        setReportColumnValue((8 * q) + 1, maxs_utop[q]);
        setReportColumnValue((8 * q) + 2, avgs_diff[q]);
        setReportColumnValue((8 * q) + 3, maxs_diff[q]);
        setReportColumnValue((8 * q) + 4, avgs_curr[q]);
        setReportColumnValue((8 * q) + 5, maxs_curr[q]);
        setReportColumnValue((8 * q) + 6, rel_areas[q]);
        setReportColumnValue((8 * q) + 7, avgs_unmet[q]);
    }
    setReportColumnValue(92, m_cells[ 910][242][(int)Layer::REVISIT_TIME_UTOPIA].value);
    setReportColumnValue(93, m_cells[ 910][242][(int)Layer::REVISIT_TIME_ACTUAL].value);
    setReportColumnValue(94, m_cells[1419][443][(int)Layer::REVISIT_TIME_UTOPIA].value);
    setReportColumnValue(95, m_cells[1419][443][(int)Layer::REVISIT_TIME_ACTUAL].value);
    setReportColumnValue(96, m_cells[ 700][ 70][(int)Layer::REVISIT_TIME_UTOPIA].value);
    setReportColumnValue(97, m_cells[ 700][ 70][(int)Layer::REVISIT_TIME_ACTUAL].value);
    setReportColumnValue(98, m_cells[ 992][619][(int)Layer::REVISIT_TIME_UTOPIA].value);
    setReportColumnValue(99, m_cells[ 992][619][(int)Layer::REVISIT_TIME_ACTUAL].value);
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
        auto cells = a->getWorldFootprint(m_world_positions);
        bool capturing = a->isCapturing();
        for(auto& c : cells) {
            updateLayer(Layer::REVISIT_TIME_UTOPIA, c.x, c.y, true);
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
        case Layer::REVISIT_TIME_UTOPIA:
        case Layer::REVISIT_TIME_ACTUAL:
            if(active) {
                cell.value = 0.f;
            } else {
                cell.value += Config::time_step;
            }
            break;
        default:
            Log::err << "Unrecognized layer " << (unsigned int)l;
            break;
    }
}
