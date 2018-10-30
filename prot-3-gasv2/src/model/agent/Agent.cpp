/***********************************************************************************************//**
 *  An agent in the world.
 *  @class      Agent
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-22
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Agent.hpp"

CREATE_LOGGER(Agent)

Agent::Agent(std::string id, sf::Vector3f init_pos, sf::Vector3f init_vel)
    : m_id(id)
    , m_self_view(id)
    , m_motion(this, init_pos, init_vel)
    , m_environment(std::make_shared<EnvModel>(this, (Config::world_width / Config::model_unity_size), (Config::world_height / Config::model_unity_size)))
    , m_link(std::make_shared<AgentLink>(this))
    , m_activities(std::make_shared<ActivityHandler>(this))
    , m_current_activity(nullptr)
    , m_display_resources(false)
{
    m_payload.setDimensions(m_environment->getEnvModelInfo());
    m_payload.setPosition(m_motion.getProjection2D());
    m_activities->setAgentId(m_id);
    initializeResources();
}

Agent::Agent(std::string id)
    : m_id(id)
    , m_self_view(id)
    , m_motion(this, Random::getUf(0.f, 2 * Config::pi))
    , m_environment(std::make_shared<EnvModel>(this, (Config::world_width / Config::model_unity_size), (Config::world_height / Config::model_unity_size)))
    , m_link(std::make_shared<AgentLink>(this))
    , m_activities(std::make_shared<ActivityHandler>(this))
    , m_current_activity(nullptr)
    , m_display_resources(false)
{
    m_payload.setDimensions(m_environment->getEnvModelInfo());
    m_payload.setPosition(m_motion.getProjection2D());
    m_activities->setAgentId(m_id);
    initializeResources();
}

void Agent::initializeResources(void)
{
    auto energy = std::make_shared<CumulativeResource>(this, "energy", 10.f);
    m_resources["energy"] = std::static_pointer_cast<Resource>(energy);
    m_resources["energy"]->addRate(Config::agent_energy_generation_rate, nullptr);  /* Constant generation of energy. */
    // auto storage = std::make_shared<CumulativeResource>(this, "storage", 10.f, 10.f);
    // m_resources["storage"] = std::static_pointer_cast<Resource>(storage);
}

void Agent::step(void)
{
    m_motion.step();
    m_payload.setPosition(m_motion.getProjection2D());

    plan();
    execute();
    consume();

    /* Update views: */
    if(m_display_resources) {
        std::stringstream ss;
        ss << m_id << ":\n";
        for(auto& r : m_resources) {
            ss << std::fixed << std::setprecision(0) << 100.f * r.second->getCapacity() / r.second->getMaxCapacity() << "\n";
        }
        m_self_view.setText(ss.str());
    } else {
        m_self_view.setText(m_id);
    }
    m_self_view.setPosition(m_motion.getProjection2D());
    m_self_view.setDirection(m_motion.getDirection2D());
    m_self_view.setFootprint(m_payload.getFootprint());
}

void Agent::plan(void)
{
    /* Schedule activities: */
    float tv_now = VirtualTime::now();
    bool resources_ok = true;
    for(auto& r : m_resources) {
        if(r.second->getCapacity() / r.second->getMaxCapacity() < 0.25f) {
            resources_ok = false;
            break;
        }
    }
    if(m_activities->pending(m_id) == 0 && resources_ok) {
        /* Create a temporal activity (won't be added to the Activities Handler): */
        float t_end = tv_now + Config::agent_planning_window * Config::time_step;
        auto tmp_act = createActivity(tv_now, t_end, m_payload.getSwath());

        /* Compute and display payoff for the temporal activity object: */
        m_environment->computePayoff(tmp_act, true);

        /* Based on previously computed payoff, generate potential activities: */
        auto act_gens = m_environment->generateActivities(tmp_act);
        std::vector<std::shared_ptr<Activity> > acts;
        for(auto& ag : act_gens) {
            acts.push_back(createActivity(ag.t0, ag.t1, m_payload.getSwath()));
        }
        float ts = acts.front()->getStartTime();
        float te = acts.back()->getEndTime();

        /* Create scheduler instance: */
        std::map<std::string, std::shared_ptr<const Resource> > rs_cpy_const(m_resources.begin(), m_resources.end());
        GAScheduler scheduler(ts, te, rs_cpy_const);

        /* Configure chromosomes: */
        std::vector<float> t0s;
        std::vector<int> steps;
        for(auto& act : acts) {
            t0s.push_back(act->getStartTime());
            steps.push_back((act->getEndTime() - act->getStartTime()) / Config::time_step);
        }
        scheduler.setChromosomeInfo(t0s, steps, m_payload.getResourceRates());

        /* Configure activity/chromosome payoffs for fitness calculation: */
        for(unsigned int i = 0; i < act_gens.size(); i++) {
            scheduler.setAggregatedPayoff(i, act_gens[i].c_coord, act_gens[i].c_payoffs, Aggregate::SUM_VALUE);
        }

        /* scheduler.debug(); */

        /* Run the scheduler: */
        auto result = scheduler.schedule();

        /* Store the result: */
        for(auto& setimes : result) {
            auto new_act = createActivity(setimes.first, setimes.second, m_payload.getSwath());
            m_activities->add(new_act);
            m_environment->addActivity(new_act);
        }
    }
}

void Agent::execute(void)
{
    /* Execute activities: */
    if(m_current_activity != nullptr) {
        /* There's an activity executing: */
        if(m_current_activity->getEndTime() <= VirtualTime::now()) {
            /* This activity has to end. */
            Log::dbg << "Agent " << m_id << " is ending activity " << m_current_activity->getId()
                << ", T = [" << m_current_activity->getStartTime() << ", " << m_current_activity->getEndTime() << ").\n";
            m_payload.disable();
            for(auto& r : m_resources) {
                // Log::warn << "Removing a resource rate: \'" << r.first << "\'.\n";
                r.second->removeRate(m_current_activity.get());
                // r.second->showStatus();
            }
            m_current_activity = nullptr;
        }
    }

    if(m_current_activity == nullptr) {
        auto actptr = m_activities->getCurrentActivity();
        if(actptr != nullptr) {
            if(actptr->getStartTime() <= VirtualTime::now()) {
                /* This activity has to start: */
                m_current_activity = actptr;
                m_current_activity->setConfirmed();
                Log::dbg << "Agent " << m_id << " is starting activity " << m_current_activity->getId() << ".\n";
                m_payload.enable();
                for(auto& r : m_resources) {
                    r.second->addRate(m_payload.getResourceRate(r.first), m_current_activity.get());
                }
            }
        }
    }
}

void Agent::consume(void)
{
    /* Update resources: */
    for(auto& r : m_resources) {
        try {
            r.second->step();
        } catch(const std::runtime_error& e) {
            Log::err << "Resource violation exception catched. Continue for debugging purposes.\n";
        }
    }
}

void Agent::showResources(bool d)
{
    m_display_resources = d;
}

std::vector<sf::Vector2i> Agent::getWorldFootprint(void) const
{
    return m_payload.getVisibleCells(true);
}

bool Agent::operator==(const Agent& ra)
{
    return (ra.getId() == getId());
}

bool Agent::operator!=(const Agent& ra)
{
    return !(*this == ra);
}

std::shared_ptr<Activity> Agent::createActivity(float t0, float t1, float swath)
{
    if(t0 >= t1 || t0 < VirtualTime::now()) {
        Log::err << "Agent " << m_id << " failed when creating activity, start and end times are wrong: " << t0 << ", " << t1 << " ["
            << (int)(t0 >= t1) << "|" << (int)(t0 < VirtualTime::now()) << "]\n";
        throw std::runtime_error("Error creating activity (1)");
    }
    unsigned int n_steps = (t1 - t0) / Config::time_step;
    unsigned int n_delay = (t0 - VirtualTime::now()) / Config::time_step;
    std::vector<sf::Vector3f> ps = m_motion.propagate(n_delay + n_steps);

    if(ps.size() != n_delay + n_steps) {
        Log::err << "Agent " << m_id << " failed when creating activity, unexpected propagation points ("
            << n_delay + n_steps << " req., " << ps.size() << " returned).\n";
        throw std::runtime_error("Error creating activity (2)");
    }

    std::map<float, sf::Vector2f> a_pos;
    std::vector<ActivityCell> a_cells;

    struct default_lut_idx {
        int v = -1;
    };
    std::map<int, std::map<int, default_lut_idx> > a_cells_lut;

    /* Find active cells and their times: */
    float t = t0;
    for(auto p = ps.begin() + n_delay; p != ps.begin() + n_delay + n_steps; p++) {
        sf::Vector2f p2d(p->x, p->y);
        a_pos[t] = sf::Vector2f(p->x, p->y);
        auto cell_coords = m_payload.getVisibleCells(swath, p2d);
        for(auto& cit : cell_coords) {
            /* Check whether that cell was already in the list: */
            int idx = a_cells_lut[cit.x][cit.y].v;
            if(idx != -1) {
                if(a_cells[idx].ready) {
                    /* Was added but T1 was already set. Create a new pair T0 and T1. */
                    float* prev_t0s = a_cells[idx].t0s;
                    float* prev_t1s = a_cells[idx].t1s;
                    int new_size = a_cells[idx].nts + 1;
                    a_cells[idx].t0s = new float[new_size];
                    a_cells[idx].t1s = new float[new_size];
                    for(unsigned int i = 0; i < a_cells[idx].nts; i++) {
                        a_cells[idx].t0s[i] = prev_t0s[i];
                        a_cells[idx].t1s[i] = prev_t1s[i];
                    }
                    a_cells[idx].nts = new_size;
                    a_cells[idx].t0s[a_cells[idx].nts - 1] = t;
                    a_cells[idx].t1s[a_cells[idx].nts - 1] = t;
                    a_cells[idx].ready = false;
                } else {
                    /* Previously added, update its t1 time: */
                    a_cells[idx].t1s[a_cells[idx].nts - 1] = t;
                }
            } else {
                /* New cell, add it now: */
                ActivityCell cell;
                cell.x  = cit.x;
                cell.y  = cit.y;
                cell.t0s = new float[1];
                cell.t1s = new float[1];
                cell.nts = 1;
                cell.t0s[0] = t;
                cell.t1s[0] = t;
                cell.ready = false;
                a_cells.push_back(cell);
                a_cells_lut[cit.x][cit.y].v = a_cells.size() - 1; /* Update look-up table. */
            }
        }
        t += Config::time_step;
    }
    return m_activities->createOwnedActivity(a_pos, a_cells);
}

void Agent::displayActivities(ActivityDisplayType af)
{
    if(af != ActivityDisplayType::NONE) {
        m_activities->autoUpdateView(true);
    } else {
        m_activities->autoUpdateView(false);
    }
    m_activities->displayInView(af);
}
