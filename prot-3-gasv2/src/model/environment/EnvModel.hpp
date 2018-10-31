/***********************************************************************************************//**
 *  Estimated state of the environment that the Agents capture and get payoff from.
 *  @class      EnvModel
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-10
 *  @version    0.2
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef ENV_MODEL_HPP
#define ENV_MODEL_HPP

#include "prot.hpp"
#include "hashers.hpp"
#include <unordered_set>
#include "World.hpp"
#include "GridView.hpp"
#include "EnvCell.hpp"
#include "PayoffFunctions.hpp"
#include "HasView.hpp"

class Agent;

struct EnvModelInfo {
    unsigned int mh;     /* Cover a number of pixels. */
    unsigned int mw;     /* Cover a number of pixels. */
    float rw;            /* Unit width in [model-units / world-pixel].  */
    float rh;            /* Unit height in [model-units / world-pixel]. */
};

struct ActivityGen {
    double t0;
    double t1;
    unsigned int steps;
    std::vector<sf::Vector2i> c_coord;
    std::vector<float> c_payoffs;
};

class EnvModel : public HasView
{
public:
    /*******************************************************************************************//**
     *  Creates an environment model of size mw x mh. For each model cell, an EnvCell object is
     *  created.
     *  @param  aptr    Pointer to the Agent that owns this EnvModel.
     *  @param  mw      Width of the environment model in number of cells.
     *  @param  mh      Height of of the environment model in number of cells.
     **********************************************************************************************/
    EnvModel(Agent* aptr, unsigned int mw, unsigned int mh);

    /*******************************************************************************************//**
     *  Compute the payoff value for some cells of the model. EnvCell::computeCellPayoff is called
     *  iteratively in a parallel pipeline (OpenMP's parallel for). Payoff is only computed for
     *  cells that are active in tmp_act (see Activity::getActiveCells). Ideally, this temporal
     *  activity (tmp_act) should be a long task that comprises the whole scheduling window of an
     *  agent. The optional argument display_in_view determines whether the resulting cell payoff
     *  value will be represented in the grid view of this model. Note that in order for this
     *  visualization to happen, the grid view has had to be enabled with EnvModel::buildView
     *  beforehand.
     *  @param  tmp_act         A temporal activity used to determine the payoff of what cells need
     *                          to be computed.
     *  @param  display_in_view Whether the payoff value will be displayed in the grid view.
     **********************************************************************************************/
    void computePayoff(std::shared_ptr<Activity> tmp_act, bool display_in_view = false);

    /*******************************************************************************************//**
     *  Split a temporal activity into multiple potential activities after payoff has been computed
     *  (with EnvModel::computePayoff). Iterates over tmp_act's active times and finds intervals
     *  where the cells' payoff is above Config::min_payoff. Based on the previous intervals this
     *  function generates, at most, Config::max_tasks tasks and ensures that their duration is
     *  shorter or equal to Config::max_task_duration.
     *  @param  tmp_act     A temporal activity used to iterate in time and generate activities that
     *                      are "sub-tasks" of tmp_act.
     *  @return A vector of activity generation information.
     *  @see ActivityGen.
     **********************************************************************************************/
    std::vector<ActivityGen> generateActivities(std::shared_ptr<Activity> tmp_act);

    /*******************************************************************************************//**
     *  Stores a newly created or received activity, to its active cells. This function should be
     *  called only for activities that are actual beliefs of an Agent (never for temporal objects).
     *  @param  act         The activity to bind to cells of this environment.
     **********************************************************************************************/
    void addActivity(std::shared_ptr<Activity> act);

    /*******************************************************************************************//**
     *  Removes a previously added activity from the cells that store it. This may be called when an
     *  activity has expired, has been discarded or is no longer an agent's beliefs.
     *  @param  act         The activity to remove.
     *  @return True if the activity was still stored within, at least, one cell. False if the
     *  activity had not been previously added or had been completely cleaned (automatically with
     *  EnvModel::cleanActivities).
     **********************************************************************************************/
    bool removeActivity(std::shared_ptr<Activity> act);

    /*******************************************************************************************//**
     *  Calls the clean function for each cell of this environment. This function automatically
     *  unbinds/removes activities that are meaningless in the context of a cell. This is partially
     *  equivalent to EnvModel::removeActivity. The main difference is that cleaning with this
     *  function is performed on a cell-basis (i.e. considering other activities present/confirmed
     *  by the cells). This function can accelerate later payoff computations because reduces the
     *  number of activities to consider in each cell.
     *  Cells are cleaned for time `t`. If this is not provided (or is equal to -1), the current
     *  virtual time will be used.
     **********************************************************************************************/
    void cleanActivities(double t = -1.f);

    EnvModelInfo getEnvModelInfo(void) const { return { m_model_h, m_model_w, m_ratio_w, m_ratio_h }; }
    unsigned int getModelWidth(void) const { return m_model_w; }
    unsigned int getModelHeight(void) const { return m_model_h; }

    std::vector<sf::Vector2i> getWorldCells(EnvCell model_cell) const;
    std::vector<sf::Vector2i> getWorldCells(unsigned int x, unsigned int y) const;
    std::vector<sf::Vector2i> getWorldCells(sf::Vector2i model_cell) const;
    std::vector<sf::Vector2i> getWorldCells(std::vector<sf::Vector2i> model_cells) const;
    template <class T>
    sf::Vector2<T> toWorldCoordinates(sf::Vector2<T> model_coord) const;
    template <class T>
    sf::Vector2<T> toModelCoordinates(sf::Vector2<T> world_coord) const;

    void buildView(void);
    void clearView(void);
    const GridView& getView(void) const;

private:
    Agent* m_agent;             /* Owner. */
    unsigned int m_model_h;     /* Cover a number of pixels. */
    unsigned int m_model_w;     /* Cover a number of pixels. */
    unsigned int m_world_h;     /* Real world magnitude (here in pixels.) */
    unsigned int m_world_w;     /* Real world magnitude (here in pixels.) */
    float m_ratio_w;            /* Unit width in [world-pixel / model-unit].  */
    float m_ratio_h;            /* Unit height in [world-pixel / model-unit]. */
    std::vector<std::vector<EnvCell> > m_cells;
    std::shared_ptr<GridView> m_payoff_view;
};

template <class T>
sf::Vector2<T> EnvModel::toWorldCoordinates(sf::Vector2<T> model_coord) const
{
    return sf::Vector2<T>(model_coord.x * m_ratio_w, model_coord.y * m_ratio_h);
}

template <class T>
sf::Vector2<T> EnvModel::toModelCoordinates(sf::Vector2<T> world_coord) const
{
    return sf::Vector2<T>(world_coord.x / m_ratio_w, world_coord.y / m_ratio_h);
}


#endif /* ENV_MODEL_HPP */
