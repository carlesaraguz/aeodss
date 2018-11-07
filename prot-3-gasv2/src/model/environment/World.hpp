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

class Agent;

class World : public TimeStep, public HasView
{
public:
    enum class Layer : unsigned int {
        COVERAGE_BEST = 0,
        COVERAGE_ACTUAL,
        REVISIT_TIME_BEST,
        REVISIT_TIME_ACTUAL,
        OVERLAPPING_ACTUAL,
        OVERLAPPING_WORST  /* NOTE n_layers (!) */
    };

    World(void);
    ~World(void);

    void addAgent(std::shared_ptr<Agent> aptr);
    void addAgent(std::vector<std::shared_ptr<Agent> > aptrs);
    void step(void) override;
    void display(Layer l);
    const GridView& getView(void) const override { return m_self_view; }
    void report(void);

    static unsigned int getWidth(void) { return m_width; }
    static unsigned int getHeight(void) { return m_height; }
    static const unsigned int n_layers = 6;

private:
    struct WorldCell {
        float value;
    };

    static unsigned int m_width;
    static unsigned int m_height;
    GridView m_self_view;
    std::vector<std::vector<std::vector<WorldCell> > > m_cells;
    std::vector<std::shared_ptr<Agent> > m_agents;

    void updateLayer(Layer l, int x, int y, bool active);
    void updateAllLayers(int x, int y, bool active);

    /* DEBUG ==================================================================================== */
    float m_mean_best_revisit_time;
    float m_min_best_revisit_time;
    float m_max_best_revisit_time;
    float m_mean_actual_revisit_time;
    float m_min_actual_revisit_time;
    float m_max_actual_revisit_time;
    float m_mean_overlapping;
    float m_max_overlapping;
    float m_worst_overlapping;
    float m_mean_best_coverage;
    float m_mean_actual_coverage;
    std::ofstream m_report_file;
    /* ========================================================================================== */
};

#endif /* WORLD_HPP */
