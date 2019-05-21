/***********************************************************************************************//**
 *  The world in which Agents actually live.
 *  @class      World
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-19
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef WORLD_HPP
#define WORLD_HPP

#include "prot.hpp"
#include "TimeStep.hpp"
#include "HasView.hpp"
#include "GridView.hpp"
#include "ReportGenerator.hpp"
#include "HeatMap.hpp"

class Agent;

class World : public TimeStep, public HasView, public ReportGenerator
{
public:
    enum class Layer : unsigned int {
        REVISIT_TIME_UTOPIA = 0,
        REVISIT_TIME_ACTUAL  /* NOTE n_layers (!) */
    };

    World(void);
    ~World(void);

    void addAgent(std::shared_ptr<Agent> aptr);
    void addAgent(std::vector<std::shared_ptr<Agent> > aptrs);
    void step(void) override;
    void display(Layer l);
    void computeMetrics(bool last = false);
    const GridView& getView(void) const override { return m_self_view; }

    static const std::vector<std::vector<sf::Vector3f> >& getPositionLUT(void) { return m_world_positions; }
    static unsigned int getWidth(void) { return m_width; }
    static unsigned int getHeight(void) { return m_height; }
    static const unsigned int n_layers = 2;

private:
    struct WorldCell {
        float value;
    };
    struct MetricsGrid {
        unsigned int x0;
        unsigned int x1;
        unsigned int y0;
        unsigned int y1;
    };

    static unsigned int m_width;
    static unsigned int m_height;
    std::vector<MetricsGrid> m_metrics_grids;
    GridView m_self_view;
    std::vector<std::vector<std::vector<WorldCell> > > m_cells;
    std::vector<std::shared_ptr<Agent> > m_agents;
    std::map<unsigned int, std::tuple<std::string, unsigned int, unsigned int, unsigned int> > m_spots; /* report Idx. -> name, m_cell indices. */
    HeatMap m_hm_max_actual;
    HeatMap m_hm_max_utopia;
    HeatMap m_hm_avg_actual;
    HeatMap m_hm_avg_utopia;
    HeatMap m_hm_count_actual;
    HeatMap m_hm_count_utopia;
    bool*** m_update_heatmaps;
    unsigned int m_delay_hm;
    unsigned int m_hm_dim_ratio_lng;
    unsigned int m_hm_dim_ratio_lat;

    static std::vector<std::vector<sf::Vector3f> > m_world_positions;  /**< Look-up table of world 3D coordinates (ECEF). */

    void updateLayer(Layer l, int x, int y, bool active, bool update_heatmaps = true);
    void updateAllLayers(int x, int y, bool active, bool update_heatmaps = true);
};

#endif /* WORLD_HPP */
