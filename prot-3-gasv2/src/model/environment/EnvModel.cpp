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
    , m_payoff_view(nullptr)
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
    if(Config::motion_model == AgentMotionType::ORBITAL) {
        m_world_positions.reserve(m_model_w);
    }
    float lat, lng;
    for(unsigned int i = 0; i < m_model_w; i++) {
        std::vector<EnvCell> column;
        std::vector<sf::Vector3f> column_lut;
        column.reserve(m_model_h);
        if(Config::motion_model == AgentMotionType::ORBITAL) {
            column_lut.reserve(m_model_h);
        }
        for(unsigned int j = 0; j < m_model_h; j++) {
            EnvCell c(i, j);
            c.pushPayoffFunc(PayoffFunctions::f_revisit_time_backwards);
            c.pushPayoffFunc(PayoffFunctions::f_revisit_time_forwards);
            column.push_back(c);
            if(Config::motion_model == AgentMotionType::ORBITAL) {
                lng =   (360.f * (i * m_ratio_w) / World::getWidth()) - 180.f;
                lat = -((180.f * (j * m_ratio_h) / World::getHeight()) - 90.f);
                auto position_ecef = CoordinateSystemUtils::fromGeographicToECEF(sf::Vector3f(lat, lng, 0.f));
                column_lut.push_back(position_ecef);
            }
        }
        m_cells.push_back(column);
        if(Config::motion_model == AgentMotionType::ORBITAL) {
            m_world_positions.push_back(column_lut);
        }
    }
}

void EnvModel::buildView(void)
{
    m_payoff_view = std::make_shared<GridView>(m_model_w, m_model_h, Config::model_unity_size, Config::model_unity_size);
    m_payoff_view->setColorGradient(Config::color_gradient_krbg);
}

void EnvModel::clearView(void)
{
    if(m_payoff_view) {
        m_payoff_view->setValue(0.f);
    }
}

void EnvModel::computePayoff(std::shared_ptr<Activity> tmp_act, bool display_in_view)
{
    Log::dbg << "Agent " << m_agent->getId() << " is computing payoff\n";
    double* t0s;
    double* t1s;
    float po;
    if(display_in_view && m_payoff_view) {
        clearView();
    }
    int nts;

    auto cells = tmp_act->getActiveCells();
    /*  #pragma omp parallel for
     *  NOTE: I don't think it is a good idea to parallelise here, because a lot of accesses to
     *  shared objects is done inside this loop (i.e. functions getCellTimes and computeCellPayoff).
     **/
    for(std::size_t i = 0; i < cells.size(); i++) {
        auto& c = cells[i];
        nts = tmp_act->getCellTimes(c.x, c.y, &t0s, &t1s);
        po = m_cells[c.x][c.y].computeCellPayoff(t0s, t1s, nts);
        if(display_in_view && m_payoff_view) {
            m_payoff_view->setValue(c.x, c.y, po);
        }
    }
    Log::dbg << "Agent " << m_agent->getId() << " has completed computing payoff\n";
}

void EnvModel::addActivity(std::shared_ptr<Activity> act)
{
    auto cells = act->getActiveCells();
    #pragma omp parallel for
    for(std::size_t i = 0; i < cells.size(); i++) {
        auto& c = cells[i];
        m_cells[c.x][c.y].addCellActivity(act);
    }
}

void EnvModel::removeActivity(std::shared_ptr<Activity> act)
{
    auto cells = act->getActiveCells();
    #pragma omp parallel for
    for(std::size_t i = 0; i < cells.size(); i++) {
        auto& c = cells[i];
        m_cells[c.x][c.y].removeCellActivity(act);
    }
}

void EnvModel::updateActivity(std::shared_ptr<Activity> act)
{
    auto cells = act->getActiveCells();
    #pragma omp parallel for
    for(std::size_t i = 0; i < cells.size(); i++) {
        auto& c = cells[i];
        m_cells[c.x][c.y].updateCellActivity(act);
    }
}


void EnvModel::cleanActivities(double t)
{
    if(t == -1.f) {
        t = VirtualTime::now();
    }
    #pragma omp parallel for
    for(unsigned int xx = 0; xx < m_model_w; xx++) {
        #pragma omp parallel for
        for(unsigned int yy = 0; yy < m_model_h; yy++) {
            m_cells[xx][yy].clean(t);
        }
    }
}

std::vector<ActivityGen> EnvModel::generateActivities(std::shared_ptr<Activity> tmp_act, std::vector<std::shared_ptr<Activity> > prev_acts)
{
    double duration = tmp_act->getEndTime() - tmp_act->getStartTime();
    Log::dbg << "[" << m_agent->getId() << "] Generating potential activities in the range t = ["
        << VirtualTime::toString(tmp_act->getStartTime()) << ", "
        << VirtualTime::toString(tmp_act->getEndTime()) << "] ==> Duration: "
        << VirtualTime::toString(duration, false) << ".\n";
    std::vector<ActivityGen> retval;

    /* Build a look-up-table for activity start times (which will be seamlesly sorted). */
    std::map<double, unsigned int> prev_act_ts;
    for(unsigned int i = 0; i < prev_acts.size(); i++) {
        prev_act_ts[prev_acts[i]->getStartTime()] = i;
    }

    /* Iterate in time steps, to generate new activities: */
    double tstart = tmp_act->getStartTime();
    double tend = tmp_act->getEndTime();
    double t, t0 = 0.0, t1;
    std::shared_ptr<Activity> p_act(nullptr);
    double p_tstart = tend; /* Initialise at the end of the scheduling window. */
    double p_tend = tend;   /* Initialise at the end of the scheduling window. */
    if(prev_act_ts.size() > 0) {
        p_act = prev_acts[prev_act_ts.begin()->second];     /* This is the first (existing) activity. */
        p_tstart = p_act->getStartTime();
        p_tend = p_act->getEndTime();
        Log::warn << "Considering an existing activity: (" << p_act->getId()
            << ")[" << VirtualTime::toString(p_tstart, true, true)
            << ", " << VirtualTime::toString(p_tend, true, true) << "]\n";
    }
    bool started_new = false;
    bool do_continue = false;
    bool within_old = false;
    std::unordered_set<sf::Vector2i, Vector2iHash> selected_cells;
    if(std::abs(tstart - p_tstart) <= Config::time_step) {
        Log::err << "Setting tstart to p_tstart\n";
        tstart = p_tstart;
        within_old = true;
    }
    double tend_it = p_tstart;
    do {
        unsigned int s_max;
        if(p_act == nullptr) {
            s_max = Config::agent_planning_window;
        } else {
            s_max = p_act->getPositionCount() - 1;
        }
        unsigned int s = 0;
        while(tstart + s * Config::time_step <= tend_it) {
            t = tstart + s * Config::time_step;
            if(within_old) {
                // Log::warn << "Iterating inside a previous activity... (" << p_act->getId() << ") T = " << VirtualTime::toString(t, true, true) << "\n";
            } else {
                // Log::warn << "Iterating outside a previous activity... T = " << VirtualTime::toString(t, true, true) << "\n";
            }
            auto cells = tmp_act->getActiveCells(t);
            /* Remove meaningless cells (i.e. those that don't provide enough payoff at time `t`.) */
            for(auto it = cells.begin(); it != cells.end(); ) {
                EnvCell& c = m_cells[it->x][it->y];
                bool remove_cell = true;
                for(auto p : c.getAllPayoffs()) {
                    if(p.second.first > Config::min_payoff) {
                        remove_cell = false;
                        break;
                    }
                }
                if(remove_cell) {
                    it = cells.erase(it);
                } else {
                    selected_cells.insert(*it);
                    it++;
                }
            }
            do_continue = ((cells.size() > 0) || within_old);
            if(started_new && (!do_continue || (t - t0) >= Config::max_task_duration || t >= tend_it || s == s_max)) {
                /* End the activity: */
                t1 = t;
                started_new = false;
                if(t1 > t0 && selected_cells.size() > 0) {
                    std::vector<sf::Vector2i> vec_selected_cells(selected_cells.begin(), selected_cells.end());
                    std::vector<float> vec_payoffs;
                    std::vector<float> vec_utility;
                    float po, ut;
                    for(auto& vsc : vec_selected_cells) {
                        /* COMBAK: Check that asking for payoff between t0 and t1 is correct: */
                        m_cells.at(vsc.x).at(vsc.y).getPayoff(((t0 + t1) / 2.f), po, ut);
                        vec_payoffs.push_back(po);
                        vec_utility.push_back(ut);
                    }
                    ActivityGen ag;
                    ag.t0 = t0;
                    ag.t1 = t1;
                    ag.c_coord   = vec_selected_cells;
                    ag.c_payoffs = vec_payoffs;
                    ag.c_utility = vec_utility;
                    if(within_old && p_act) {
                        Log::warn << "New activity (within old, " << p_act->getId()
                            << ") --> [" << VirtualTime::toString(t0, true, true)
                            << ", " << VirtualTime::toString(t1, true, true) << "]\n";
                        ag.prev_act = p_act;
                    } else {
                        Log::warn << "New activity --> ["
                            << VirtualTime::toString(t0, true, true)
                            << ", " << VirtualTime::toString(t1, true, true) << "]\n";
                    }
                    retval.push_back(ag);
                }
                selected_cells.clear();
            }
            if(!started_new && cells.size() > 0) {
                /* Start a new activity: */
                t0 = t;
                started_new = true;
            }
            if(retval.size() >= Config::max_tasks && !within_old) {
                break;
            }
            s++;
        }
        if(retval.size() >= Config::max_tasks && !within_old) {
            break;
        }
        /* We have reached tend_it. Update p_tstart and p_tend. */
        tstart = tend_it;
        if(within_old) {
            within_old = false;
            /* We were in and reached tend_it = p_tend. Update p_tstart and p_tend: */
            if(prev_act_ts.size() > 1) {
                prev_act_ts.erase(prev_act_ts.begin());
                p_act = prev_acts[prev_act_ts.begin()->second];
                p_tstart = p_act->getStartTime();
                p_tend = p_act->getEndTime();
                Log::warn << "Considering an existing activity: (" << p_act->getId()
                    << ")[" << VirtualTime::toString(p_tstart, true, true)
                    << ", " << VirtualTime::toString(p_tend, true, true) << "]\n";
            } else {
                Log::warn << "There aren't more existing activities to consider.\n";
                p_act = nullptr;
                p_tstart = tend;
                p_tend = tend;
            }
            tend_it = p_tstart;
        } else {
            within_old = true;
            tend_it = p_tend;   /* No need to update p_tstart and p_tend yet. */
        }
    } while(tend_it <= tend);

    return retval;
}

const GridView& EnvModel::getView(void) const
{
    if(m_payoff_view == nullptr) {
        Log::err << "Environment view for agent " << m_agent->getId() << " has not been initialized but has just been requested.\n";
        throw std::runtime_error("Uninitialized environment view requested");
    }
    return *m_payoff_view;
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
