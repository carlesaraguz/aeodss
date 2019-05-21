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
    , m_hm_max_actual(std::string("heatmap_max_actual.csv"), Aggregate::MAX_VALUE)
    , m_hm_max_utopia(std::string("heatmap_max_utopia.csv"), Aggregate::MAX_VALUE)
    , m_hm_avg_actual(std::string("heatmap_avg_actual.csv"), Aggregate::MEAN_VALUE)
    , m_hm_avg_utopia(std::string("heatmap_avg_utopia.csv"), Aggregate::MEAN_VALUE)
    , m_hm_count_actual(std::string("heatmap_count_actual.csv"), Aggregate::COUNT)
    , m_hm_count_utopia(std::string("heatmap_count_utopia.csv"), Aggregate::COUNT)
    , m_delay_hm(0)
{
    /* Prepare HeatMap control variables: */
    unsigned int hm_dim_lng = HeatMap::getLongitudeDimension();
    unsigned int hm_dim_lat = HeatMap::getLatitudeDimension();
    m_hm_dim_ratio_lng = m_width  / hm_dim_lng;
    m_hm_dim_ratio_lat = m_height / hm_dim_lat;
    m_update_heatmaps = new bool**[hm_dim_lng];
    for(unsigned int xx = 0; xx < hm_dim_lng; xx++) {
        m_update_heatmaps[xx] = new bool*[hm_dim_lat];
        for(unsigned int yy = 0; yy < hm_dim_lat; yy++) {
            m_update_heatmaps[xx][yy] = new bool[n_layers];
            for(unsigned int ll = 0; ll < n_layers; ll++) {
                m_update_heatmaps[xx][yy][ll] = true;
            }
        }
    }

    unsigned int wg = m_width / 4;
    unsigned int hg = m_height / 6;
    m_metrics_grids.push_back({ (0 * wg), (4 * wg), (0 * hg), (1 * hg) });  /* Q0 */
    m_metrics_grids.push_back({ (0 * wg), (2 * wg), (1 * hg), (2 * hg) });  /* Q1 */
    m_metrics_grids.push_back({ (2 * wg), (4 * wg), (1 * hg), (2 * hg) });  /* Q2 */
    m_metrics_grids.push_back({ (0 * wg), (1 * wg), (2 * hg), (3 * hg) });  /* Q3 */
    m_metrics_grids.push_back({ (1 * wg), (2 * wg), (2 * hg), (3 * hg) });  /* Q4 */
    m_metrics_grids.push_back({ (2 * wg), (3 * wg), (2 * hg), (3 * hg) });  /* Q5 */
    m_metrics_grids.push_back({ (3 * wg), (4 * wg), (2 * hg), (3 * hg) });  /* Q6 */
    m_metrics_grids.push_back({ (0 * wg), (1 * wg), (3 * hg), (4 * hg) });  /* Q7 */
    m_metrics_grids.push_back({ (1 * wg), (2 * wg), (3 * hg), (4 * hg) });  /* Q8 */
    m_metrics_grids.push_back({ (2 * wg), (3 * wg), (3 * hg), (4 * hg) });  /* Q9 */
    m_metrics_grids.push_back({ (3 * wg), (4 * wg), (3 * hg), (4 * hg) });  /* Q10 */
    m_metrics_grids.push_back({ (0 * wg), (2 * wg), (4 * hg), (5 * hg) });  /* Q11 */
    m_metrics_grids.push_back({ (2 * wg), (4 * wg), (4 * hg), (5 * hg) });  /* Q12 */
    m_metrics_grids.push_back({ (0 * wg), (4 * wg), (5 * hg), (6 * hg) });  /* Q13 */
    m_metrics_grids.push_back({ (0 * wg), (4 * wg), (0 * hg), (6 * hg) });  /* Global. */

    for(unsigned int i = 0; i < m_metrics_grids.size(); i++) {
        addReportColumn("utopia_avg" + std::to_string(i));          /* i x 0 */
        addReportColumn("utopia_max" + std::to_string(i));          /* i x 1 */
        addReportColumn("diff_avg" + std::to_string(i));            /* i x 2 */
        addReportColumn("diff_max" + std::to_string(i));            /* i x 3 */
        addReportColumn("actual_avg" + std::to_string(i));          /* i x 4 */
        addReportColumn("actual_max" + std::to_string(i));          /* i x 5 */
        addReportColumn("coverage_actual" + std::to_string(i));     /* i x 6 */
        addReportColumn("coverage_actual_avg" + std::to_string(i)); /* i x 7 */
        addReportColumn("coverage_utopia" + std::to_string(i));     /* i x 8 */
        addReportColumn("coverage_utopia_avg" + std::to_string(i)); /* i x 9 */
    }
    m_spots[addReportColumn("barcelona_utopia")] = std::make_tuple("barcelona_utopia",  910, 242, (int)Layer::REVISIT_TIME_UTOPIA);
    m_spots[addReportColumn("barcelona_actual")] = std::make_tuple("barcelona_actual",  910, 242, (int)Layer::REVISIT_TIME_ACTUAL);
    m_spots[addReportColumn("singapore_utopia")] = std::make_tuple("singapore_utopia", 1419, 443, (int)Layer::REVISIT_TIME_UTOPIA);
    m_spots[addReportColumn("singapore_actual")] = std::make_tuple("singapore_actual", 1419, 443, (int)Layer::REVISIT_TIME_ACTUAL);
    m_spots[addReportColumn("greenland_utopia")] = std::make_tuple("greenland_utopia",  700,  70, (int)Layer::REVISIT_TIME_UTOPIA);
    m_spots[addReportColumn("greenland_actual")] = std::make_tuple("greenland_actual",  700,  70, (int)Layer::REVISIT_TIME_ACTUAL);
    m_spots[addReportColumn("cape_town_utopia")] = std::make_tuple("cape_town_utopia",  992, 619, (int)Layer::REVISIT_TIME_UTOPIA);
    m_spots[addReportColumn("cape_town_actual")] = std::make_tuple("cape_town_actual",  992, 619, (int)Layer::REVISIT_TIME_ACTUAL);
    enableReport();

    m_cells.reserve(m_width);
    for(unsigned int i = 0; i < m_width; i++) {
        std::vector<std::vector<WorldCell> > column;
        column.reserve(m_height);
        for(unsigned int j = 0; j < m_height; j++) {
            std::vector<WorldCell> layers(n_layers, { 0.f });
            /* Initialize values for each layer: */
            layers[(int)Layer::REVISIT_TIME_UTOPIA].value = -1.f;
            layers[(int)Layer::REVISIT_TIME_ACTUAL].value = 0.f;
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

World::~World(void)
{
    unsigned int hm_dim_lng = HeatMap::getLongitudeDimension();
    unsigned int hm_dim_lat = HeatMap::getLatitudeDimension();
    for(unsigned int xx = 0; xx < hm_dim_lng; xx++) {
        for(unsigned int yy = 0; yy < hm_dim_lat; yy++) {
            delete[] m_update_heatmaps[xx][yy];
        }
        delete[] m_update_heatmaps[xx];
    }
    delete[] m_update_heatmaps;
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

void World::computeMetrics(bool last)
{
    std::vector<float> avgs_utop(m_metrics_grids.size());
    std::vector<float> avgs_diff(m_metrics_grids.size());
    std::vector<float> avgs_curr(m_metrics_grids.size());
    std::vector<float> maxs_utop(m_metrics_grids.size());
    std::vector<float> maxs_diff(m_metrics_grids.size());
    std::vector<float> maxs_curr(m_metrics_grids.size());
    std::vector<float> unmet_coverage_utop(m_metrics_grids.size());
    std::vector<float> unmet_coverage_curr(m_metrics_grids.size());
    std::vector<float> unmet_avg_utop(m_metrics_grids.size());
    std::vector<float> unmet_avg_curr(m_metrics_grids.size());
    for(unsigned int q = 0; q < m_metrics_grids.size(); q++) {
        unsigned int x0, x1, y0, y1;
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
        float curr_unmet_val = 0.f;
        float utop_unmet_val = 0.f;
        float max_val_utop = 0.f;
        float max_val_diff = 0.f;
        float max_val_curr = 0.f;
        int count_cells = 1;
        unmet_coverage_utop[q] = 0.f;
        unmet_coverage_curr[q] = 0.f;
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
                        unmet_coverage_curr[q] += 1.f;
                        curr_unmet_val += curr_val;
                    }
                    if(utop_val > Config::goal_target) {
                        unmet_coverage_utop[q] += 1.f;
                        utop_unmet_val += utop_val;
                    }
                }
            }
        }
        if(unmet_coverage_curr[q] == 0.f) {
            curr_unmet_val = 0.f;                       /* Leaves as 0. */
        } else {
            curr_unmet_val /= unmet_coverage_curr[q];   /* Does the average. */
        }
        if(unmet_coverage_utop[q] == 0.f) {
            utop_unmet_val = 0.f;                       /* Leaves as 0. */
        } else {
            utop_unmet_val /= unmet_coverage_utop[q];   /* Does the average. */
        }
        avg_val_utop /= (float)(count_cells);
        avg_val_diff /= (float)(count_cells);
        avg_val_curr /= (float)(count_cells);
        unmet_coverage_curr[q] /= (float)(count_cells);
        unmet_coverage_utop[q] /= (float)(count_cells);
        avgs_utop[q] = avg_val_utop;
        avgs_diff[q] = avg_val_diff;
        avgs_curr[q] = avg_val_curr;
        maxs_utop[q] = max_val_utop;
        maxs_diff[q] = max_val_diff;
        maxs_curr[q] = max_val_curr;
        unmet_avg_utop[q] = utop_unmet_val;
        unmet_avg_curr[q] = curr_unmet_val;
    }
    for(unsigned int q = 0; q < m_metrics_grids.size(); q++) {
        setReportColumnValue((10 * q) + 0, avgs_utop[q]);
        setReportColumnValue((10 * q) + 1, maxs_utop[q]);
        setReportColumnValue((10 * q) + 2, avgs_diff[q]);
        setReportColumnValue((10 * q) + 3, maxs_diff[q]);
        setReportColumnValue((10 * q) + 4, avgs_curr[q]);
        setReportColumnValue((10 * q) + 5, maxs_curr[q]);
        setReportColumnValue((10 * q) + 6, unmet_coverage_curr[q]);
        setReportColumnValue((10 * q) + 7, unmet_avg_curr[q]);
        setReportColumnValue((10 * q) + 8, unmet_coverage_utop[q]);
        setReportColumnValue((10 * q) + 9, unmet_avg_utop[q]);
    }
    for(auto& s : m_spots) {
        setReportColumnValue(
            s.first,
            (float)m_cells[std::get<1>(s.second)][std::get<2>(s.second)][std::get<3>(s.second)].value
        );
    }
    m_delay_hm++;
    if(m_delay_hm % 100 == 0 && !last) {
        Log::dbg << "Refreshing heatmaps...\n";
        m_hm_max_actual.saveHeatMap();
        m_hm_max_utopia.saveHeatMap();
        m_hm_avg_actual.saveHeatMap();
        m_hm_avg_utopia.saveHeatMap();
        m_hm_count_actual.saveHeatMap();
        m_hm_count_utopia.saveHeatMap();
    } else if(last) {
        Log::dbg << "Saving heat map \'" << m_hm_max_actual.getReportFilename() << "\'\n";
        m_hm_max_actual.saveHeatMap();
        Log::dbg << "Saving heat map \'" << m_hm_max_utopia.getReportFilename() << "\'\n";
        m_hm_max_utopia.saveHeatMap();
        Log::dbg << "Saving heat map \'" << m_hm_avg_actual.getReportFilename() << "\'\n";
        m_hm_avg_actual.saveHeatMap();
        Log::dbg << "Saving heat map \'" << m_hm_avg_utopia.getReportFilename() << "\'\n";
        m_hm_avg_utopia.saveHeatMap();
        Log::dbg << "Saving heat map \'" << m_hm_count_actual.getReportFilename() << "\'\n";
        m_hm_count_actual.saveHeatMap();
        Log::dbg << "Saving heat map \'" << m_hm_count_utopia.getReportFilename() << "\'\n";
        m_hm_count_utopia.saveHeatMap();
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
            updateAllLayers(xx, yy, false, false);
        }
    }
    for(auto& a : m_agents) {
        auto cells = a->getWorldFootprint(m_world_positions);
        bool capturing = a->isCapturing();
        for(auto& c : cells) {
            updateLayer(Layer::REVISIT_TIME_UTOPIA, c.x, c.y, true, true);
            updateLayer(Layer::REVISIT_TIME_ACTUAL, c.x, c.y, capturing, true);
        }
    }
}

void World::updateAllLayers(int x, int y, bool active, bool update_heatmaps)
{
    for(unsigned int l = 0; l < n_layers; l++) {
        updateLayer(static_cast<Layer>(l), x, y, active, update_heatmaps);
    }
}

void World::updateLayer(Layer l, int x, int y, bool active, bool update_heatmaps)
{
    auto& cell = m_cells[x][y][(int)l];
    unsigned int hm_x = x / m_hm_dim_ratio_lng;
    unsigned int hm_y = y / m_hm_dim_ratio_lat;
    bool is_heatmap_pixel = (x % m_hm_dim_ratio_lng == 0) && (y % m_hm_dim_ratio_lat == 0);
    switch(l) {
        case Layer::REVISIT_TIME_UTOPIA:
            if(active) {
                /*  We don't save values that are currently being accessed:
                 **/
                if(hm_x == 44 && hm_y == 109 && is_heatmap_pixel) {
                    Log::dbg << "[" << update_heatmaps << "] Updating heatmap: " << m_update_heatmaps[hm_x][hm_y][(int)l] << "\n";
                }
                if(cell.value != 0.f && is_heatmap_pixel && m_update_heatmaps[hm_x][hm_y][(int)l]) {
                    double rt = cell.value;
                    if(rt < 0.f) {
                        rt = VirtualTime::now() - Config::start_epoch;
                    }
                    m_hm_max_utopia.setRevisitTime(hm_x, hm_y, rt);
                    m_hm_avg_utopia.setRevisitTime(hm_x, hm_y, rt);
                    m_hm_count_utopia.setRevisitTime(hm_x, hm_y, rt);
                    m_update_heatmaps[hm_x][hm_y][(int)l] = false;
                }
                cell.value = 0.f;
            } else if(cell.value >= 0.f) {
                if(cell.value > 0.f) {
                    m_update_heatmaps[hm_x][hm_y][(int)l] = true;
                }
                cell.value += Config::time_step;
            }
        break;
        case Layer::REVISIT_TIME_ACTUAL:
            if(active) {
                /*  Similar to the utopia case. We don't save locations that are currently being
                 *  accessed.
                 **/
                if(cell.value > 0.f && is_heatmap_pixel && m_update_heatmaps[hm_x][hm_y][(int)l]) {
                    m_hm_max_actual.setRevisitTime(hm_x, hm_y, cell.value);
                    m_hm_avg_actual.setRevisitTime(hm_x, hm_y, cell.value);
                    m_hm_count_actual.setRevisitTime(hm_x, hm_y, cell.value);
                    m_update_heatmaps[hm_x][hm_y][(int)l] = false;
                }
                cell.value = 0.f;
            } else if(cell.value >= 0.f) {
                if(cell.value > 0.f) {
                    m_update_heatmaps[hm_x][hm_y][(int)l] = true;
                }
                cell.value += Config::time_step;
            }
            break;
        default:
            Log::err << "Unrecognized layer " << (unsigned int)l;
            break;
    }
}
