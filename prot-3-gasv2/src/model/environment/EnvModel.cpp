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
            EnvCell c(m_agent, i, j);
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
    for(std::size_t i = 0; i < cells.size(); i++) {
        auto& c = cells[i];
        m_cells[c.x][c.y].addCellActivity(act);
    }
}

void EnvModel::removeActivity(std::shared_ptr<Activity> act)
{
    auto cells = act->getActiveCells();
    /* We get the pointer once, since it was added for all cells at the same time and should be the same. */
    std::shared_ptr<Activity> real_aptr;
    if(cells.size() > 0) {
        real_aptr = m_cells[cells[0].x][cells[0].y].getActivity(act->getAgentId(), act->getId());
    }
    for(std::size_t i = 0; i < cells.size(); i++) {
        auto& c = cells[i];
        m_cells[c.x][c.y].removeCellActivity(real_aptr);
    }
}

void EnvModel::updateActivity(std::shared_ptr<Activity> act)
{
    auto cells = act->getActiveCells();
    bool updated = false;
    for(std::size_t i = 0; i < cells.size(); i++) {
        auto& c = cells[i];
        auto aptr = m_cells[c.x][c.y].getActivity(act->getAgentId(), act->getId());
        if(aptr != nullptr) {
            /*  The environment model has this activity. Update it here, once.
             *  NOTE: Cells hold a pointer to activities. Therefore, updating the activity once
             *  suffices (as long as the inner function EnvCell::updateCellActivity only performs an
             *  Activity::clone operation). Note that the pointer/object stored in EnvCell has been
             *  added with EnvCell::addCellActivity but may not be the same than `act`. However,
             *  since activities are added to all cells at once, the pointers are certainly shared
             *  across cells (and because of this, we can find them and update them once).
             **/
            aptr->clone(act);
            updated = true;
            break;
        }
    }
    if(!updated) {
        Log::err << "Unable to update activity [" << act->getAgentId() << ":" << act->getId() << "] in the environment model of "
            << m_agent->getId() << ". The agent does not remember this activity.\n";
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

std::set<std::pair<std::string, unsigned int> > EnvModel::getCrosscheckList(void) const
{
    std::set<std::pair<std::string, unsigned int> > retset;
    for(unsigned int xx = 0; xx < m_model_w; xx++) {
        for(unsigned int yy = 0; yy < m_model_h; yy++) {
            auto cellset = m_cells[xx][yy].getCellCrosscheckList();
            for(auto& cellpair : cellset) {
                retset.insert(cellpair);
            }
        }
    }
    return retset;
}


ActivityGen EnvModel::createActivityGen(double t0, double t1, std::shared_ptr<Activity> tmp_aptr, std::shared_ptr<Activity> aptr)
{
    ActivityGen ag;
    ag.t0 = t0;
    ag.t1 = t1;
    ag.prev_act = aptr;

    double steps_exact = (t1 - t0) / Config::time_step;
    if(steps_exact < 2.0) {
        Log::warn << "Skipped the creation of one activity with too short duration\n";
        ag.valid = false;
        return ag;
    }

    std::unordered_set<sf::Vector2i, Vector2iHash> unique_cells;
    double t = t0;
    while(t <= t1) {
        auto cells = tmp_aptr->getActiveCells(t);
        for(auto it = cells.begin(); it != cells.end(); it++) {
            unique_cells.insert(*it);   /* This is a set, it prevents repeated cells. */
        }
        t += Config::time_step;
    }
    std::vector<sf::Vector2i> vcells(unique_cells.begin(), unique_cells.end()); /* Copies to vector. */
    std::vector<float> vec_payoffs;
    std::vector<float> vec_utility;
    float po, ut;
    for(auto& vsc : vcells) {
        /* COMBAK: Check that asking for payoff between t0 and t1 is correct: */
        m_cells.at(vsc.x).at(vsc.y).getPayoff(((t0 + t1) / 2.f), po, ut);
        vec_payoffs.push_back(po);
        vec_utility.push_back(ut);
    }
    ag.c_coord   = vcells;
    ag.c_payoffs = vec_payoffs;
    ag.c_utility = vec_utility;
    /* DEBUG with:
    Log::warn << "Activity generator is ready for [" << VirtualTime::toString(t0) << "," << VirtualTime::toString(t1) << "]. ";
    if(aptr != nullptr) {
        Log::warn << "Act: [" << aptr->getAgentId() << ":" << aptr->getId() << "]";
    }
    Log::warn << "\n";
    */
    return ag;
}

double EnvModel::findEndTime(double t0, double max_t1, std::shared_ptr<Activity> aptr, double& next_t0)
{
    /* Look for t1: */
    double t = t0;
    for(unsigned int s = 0; s < aptr->getPositionCount(); s++) {
        t = t0 + s * Config::time_step;
        if(t >= max_t1) {
            t = max_t1;
            break;
        }
        auto cells = aptr->getActiveCells(t);
        /* Remove meaningless cells (i.e. those that don't provide enough payoff at time `t`.) */
        for(auto it = cells.begin(); it != cells.end(); ) {
            EnvCell& c = m_cells[it->x][it->y];
            bool remove_cell = true;
            for(auto p : c.getAllPayoffs()) {
                if(p.second.first >= Config::min_payoff || Config::mode == SandboxMode::RANDOM) {
                    remove_cell = false;
                    break;
                }
            }
            if(remove_cell) {
                it = cells.erase(it);
            } else {
                it++;
            }
        }
        if(cells.size() == 0) {
            break;
        }
    }
    /* Exit for-loop: found t1 = t. */
    double t1 = std::min(t, max_t1);    /* Just in case we exceeded aptr end time accidentally. */

    /* Look for next t0: */
    if(t1 + Config::time_step >= max_t1) {
        next_t0 = max_t1;
    } else {
        t = t1;
        for(unsigned int s = 0; s < aptr->getPositionCount(); s++) {
            t = t1 + s * Config::time_step;
            if(t >= aptr->getEndTime()) {
                t = aptr->getEndTime();
                break;
            }
            auto cells = aptr->getActiveCells(t);
            /* Remove meaningless cells (i.e. those that don't provide enough payoff at time `t`.) */
            for(auto it = cells.begin(); it != cells.end(); ) {
                EnvCell& c = m_cells[it->x][it->y];
                bool remove_cell = true;
                for(auto p : c.getAllPayoffs()) {
                    if(p.second.first >= Config::min_payoff || Config::mode == SandboxMode::RANDOM) {
                        remove_cell = false;
                        break;
                    }
                }
                if(remove_cell) {
                    it = cells.erase(it);
                } else {
                    it++;
                }
            }
            if(cells.size() > 0) {
                break;
            }
        }
        /* Found next t0: next_t0 = t. */
        next_t0 = t;
    }
    return t1;
}

std::vector<ActivityGen> EnvModel::generateActivities(std::shared_ptr<Activity> tmp_act, std::vector<std::shared_ptr<Activity> > prev_acts)
{
    double duration = tmp_act->getEndTime() - tmp_act->getStartTime();
    Log::dbg << "[" << m_agent->getId() << "] Generating potential activities in the range t = ["
        << VirtualTime::toString(tmp_act->getStartTime()) << ", "
        << VirtualTime::toString(tmp_act->getEndTime()) << "] ==> Duration: "
        << VirtualTime::toString(duration, false) << ".\n";
    std::vector<ActivityGen> retval;

    /*  Build a look-up-table for activity start times (which will be seamlesly sorted).
     *  While doing so, ensure that tasks are not overlapping. If they are, abort.
     *  This is how the LUT is built:
     *      double(1) -> start time,
     *      double(2) -> end time,
     *      unsigned int(3) -> prev_acts idx.
     **/
    std::vector<std::tuple<double, double, unsigned int> > t_horizons;
    for(unsigned int i = 0; i < prev_acts.size(); i++) {
        t_horizons.push_back(std::make_tuple(prev_acts[i]->getStartTime(), prev_acts[i]->getEndTime(), i));
        if(i > 0) {
            /* Check overlaps: */
            if(prev_acts[i]->getStartTime() < prev_acts[i - 1]->getEndTime()) {
                /* Overlaps. Aborting. */
                Log::err << "Error generating new activties [#1a]: the provided schedule that has overlapping intervals. Aborting.\n";
                return std::vector<ActivityGen>();
            }
        }
        if(prev_acts[i]->getEndTime() - prev_acts[i]->getStartTime() < Config::time_step) {
            /* Abnormally short. Aborting. */
            Log::err << "Error generating new activties [#1b]: the provided schedule has abnormally short tasks. Aborting.\n";
            Log::err << "Err. act.: " << *prev_acts[i] << "\n";
            return std::vector<ActivityGen>();
        }
    }
    /* Sort the LUT by start time: */
    std::sort(
        t_horizons.begin(),
        t_horizons.end(),
        [](std::tuple<double, double, unsigned int> a, std::tuple<double, double, unsigned int> b) {
            return std::get<0>(a) < std::get<0>(b);
        }
    );
    /* DEBUG with:
    Log::dbg << "Listing horizons:\n";
    for(auto& th : t_horizons) {
        Log::dbg << " -- time: "
            << VirtualTime::toString(std::get<0>(th)) << " / "
            << VirtualTime::toString(std::get<1>(th)) << ", task: "
            << std::get<2>(th) << "\n";
    }
    */

    bool within_old = false;
    double next_horizon;
    double t_start = tmp_act->getStartTime();
    double t_end = tmp_act->getEndTime();
    if(t_horizons.size() > 0) {
        next_horizon = std::get<0>(t_horizons.front());
    } else {
        next_horizon = t_end;
    }
    double t = t_start;
    double t_start_i = t_start;
    double t_end_i = std::min(t + (Config::max_task_duration * Config::time_step), next_horizon);
    do {
        bool created_activity = false;
        while(t_end_i - t > 3.0 * Config::time_step) {
            /* Length is appropiate: */
            if(within_old) {
                int idx_lut = std::get<2>(t_horizons.front());
                auto aptr = prev_acts[idx_lut];
                if(aptr->isConfimed()) {
                    t_end_i = std::get<1>(t_horizons.front());  /* Move directly to end time, which is next horizon. */
                    auto ag = createActivityGen(t, t_end_i, tmp_act, aptr);
                    if(ag.valid) {
                        retval.push_back(ag);
                    }
                } else {
                    /* We can split. */
                    auto ag = createActivityGen(t, t_end_i, tmp_act, aptr);
                    if(ag.valid) {
                        retval.push_back(ag);
                    }
                }
                t = t_end_i;
                t_end_i = std::min(t + (Config::max_task_duration * Config::time_step), next_horizon);
                created_activity = true;
            } else {
                /* We need to iterate over time to detect earlier stopping points: */
                t_end_i = findEndTime(t, t_end_i, tmp_act, t_start_i);  /* May be left unchanged. */
                if(t_end_i - t > 3.0 * Config::time_step) {
                    /* Length continues to be appropiate. */
                    auto ag = createActivityGen(t, t_end_i, tmp_act, nullptr);
                    if(ag.valid) {
                        retval.push_back(ag);
                    }
                }
                if(t_start_i < next_horizon) {
                    t = t_start_i;
                    t_end_i = std::min(t + (Config::max_task_duration * Config::time_step), next_horizon);
                } else {
                    /* We exit the loop but do not enter the following if-clause because within_old = false. */
                    t = next_horizon;
                    t_end_i = next_horizon;
                }
                if(t_horizons.size() == 0 && !within_old && retval.size() >= Config::max_tasks) {
                    /* Exit this loop. This is safe only in this conditions. */
                    break;
                }
            }
        }
        /*  Exited the loop because:
         *   - There was a too short task; or
         *   - We reached next_horizon.
         **/
        if(!created_activity && within_old) {
            /* This is not correct. We must create (at least) one activity: */
            int idx_lut = std::get<2>(t_horizons.front());
            auto aptr = prev_acts[idx_lut];
            auto ag = createActivityGen(std::get<0>(t_horizons.front()), std::get<1>(t_horizons.front()), tmp_act, aptr);
            if(ag.valid) {
                retval.push_back(ag);
            }
        }
        if(within_old && retval.size() > 0) {
            /* Extend the previous (if there is one): */
            retval[retval.size() - 1].t1 = next_horizon;
        } else if(within_old) {
            /* This is unexpected. */
            Log::err << "Error generating new activties [#2]\n";
            return std::vector<ActivityGen>();
        }
        t = next_horizon;
        if(!within_old) {
            if(t_horizons.size() > 0) {
                /* We reached an activity start time. Set next horizon to end time: */
                next_horizon = std::get<1>(t_horizons.front());
                within_old = true;
            } /* ... else: we must have reached `t_end`, so there's nothing to do. */
        } else {
            /* We reached an activity end time. Advance LUT and set the next horizon to next start time. */
            if(t_horizons.size() > 0) {
                t_horizons.erase(t_horizons.begin());
                if(t_horizons.size() > 0) {
                    /* Next horizon is next start time: */
                    next_horizon = std::get<0>(t_horizons.front());
                } else {
                    /* There aren't more horizons: */
                    next_horizon = t_end;
                }
                within_old = false;
            } else {
                /* This is unexpected because we were inside a task. */
                Log::err << "Error generating new activities [#3]\n";
            }
        }
        t_end_i = std::min(t + (Config::max_task_duration * Config::time_step), next_horizon);

        if(!within_old && retval.size() >= Config::max_tasks) {
            /* We need to stop here: */
            break;
        } else if(within_old && std::ceil((next_horizon - t) / (Config::max_task_duration * Config::time_step)) + retval.size() >= Config::max_tasks) {
            /* We also need to stop here: */
            break;
        }
    } while(t_horizons.size() >= 1 || (!within_old && t_end - t > 3.0 * Config::time_step));

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
