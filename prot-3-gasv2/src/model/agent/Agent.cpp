/***********************************************************************************************//**
 *  An agent in the world.
 *  @class      Agent
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *              Marc Closa (MC), marc.closa.tarres@alu-etsetb.upc.edu
 *  @date       2018-nov-15
 *  @version    0.2
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Agent.hpp"

CREATE_LOGGER(Agent)

Agent::Agent(std::string id, sf::Vector2f init_pos, sf::Vector2f init_vel)
    : ReportGenerator("agents/" + id + "/", std::string("state.csv"))
    , m_id(id)
    , m_self_view(id)
    , m_motion(this, {init_pos.x, init_pos.y, 0.f}, {init_vel.x, init_vel.y, 0.f})
    , m_environment(std::make_shared<EnvModel>(this, (Config::world_width / Config::model_unity_size), (Config::world_height / Config::model_unity_size)))
    , m_link(std::make_shared<AgentLink>(this))
    , m_activities(std::make_shared<ActivityHandler>(this))
    , m_current_activity(nullptr)
    , m_display_resources(false)
    , m_link_energy_available(false)
{
    configAgentReport();
    if(Config::motion_model == AgentMotionType::ORBITAL) {
        Log::err << "Constructing agent objects with wrong arguments (2-d, linear motion).\n";
        Log::err << "The world is modelled in 3-d (i.e. `motion_model` is set to AgentMotionType::ORBITAL). Please use a different constructor.\n";
        throw std::runtime_error("Wrong Agent constructor");
    }

    /* 2-D mode / LINEAR constructor: */
    m_motion.debug();
    m_payload.setDimensions(m_environment->getEnvModelInfo());
    m_payload.setPosition(m_motion.getPosition());
    m_activities->setAgentId(m_id);
    m_activities->setEnvironment(m_environment);
    m_activities->setInstrumentAperture(m_payload.getAperture());
    m_link->setEncounterCallback([this](std::string aid) -> bool { return encounter(aid); });
    m_link->setConnectedCallback([this](std::string aid) { return connected(aid); });
    initializeResources();
}

Agent::Agent(std::string id)
    : ReportGenerator("agents/" + id + "/", std::string("state.csv"))
    , m_id(id)
    , m_self_view(id)
    , m_motion(this, -1.0)
    , m_environment(std::make_shared<EnvModel>(this, (Config::world_width / Config::model_unity_size), (Config::world_height / Config::model_unity_size)))
    , m_link(std::make_shared<AgentLink>(this))
    , m_activities(std::make_shared<ActivityHandler>(this))
    , m_current_activity(nullptr)
    , m_display_resources(false)
    , m_link_energy_available(false)
{
    configAgentReport();
    if(Config::motion_model != AgentMotionType::ORBITAL) {
        Log::err << "Constructing agent objects with wrong arguments (3-d, orbital motion).\n";
        Log::err << "The world is modelled in 2-d (i.e. `motion_model` is not set to AgentMotionType::ORBITAL). Please use a different constructor.\n";
        throw std::runtime_error("Wrong Agent constructor");
    }

    /* 3-D mode / ORBITAL constructor: */
    m_motion.debug();
    m_payload.setAperture(Random::getUf(Config::agent_aperture_min, Config::agent_aperture_max), m_motion.getMaxAltitude());
    m_payload.setDimensions(m_environment->getEnvModelInfo());
    m_payload.setPosition(m_motion.getPosition());
    m_activities->setAgentId(m_id);
    m_activities->setEnvironment(m_environment);
    m_activities->setInstrumentAperture(m_payload.getAperture());
    m_link->setEncounterCallback([this](std::string aid) -> bool { return encounter(aid); });
    m_link->setConnectedCallback([this](std::string aid) { return connected(aid); });
    initializeResources();
}

void Agent::configAgentReport(void)
{
    addReportColumn("energy");      /* 0 */
    enableReport();
}

void Agent::updateAgentReport(void)
{
    setReportColumnValue(0, std::to_string(m_resources["energy"]->getCapacity()));
}

void Agent::initializeResources(void)
{
    auto energy = std::make_shared<CumulativeResource>(this, "energy", 10.f);
    m_resources["energy"] = std::static_pointer_cast<Resource>(energy);
    m_resources["energy"]->addRate(Config::agent_energy_generation_rate, nullptr);  /* Constant generation of energy. */
    // auto storage = std::make_shared<CumulativeResource>(this, "storage", 10.f, 10.f);
    // m_resources["storage"] = std::static_pointer_cast<Resource>(storage);
}

void Agent::updatePosition(void)
{
    m_motion.step();
    m_payload.setPosition(m_motion.getPosition());
    m_link->setPosition(m_motion.getPosition());
}

void Agent::step(void)
{
    m_link->update();
    m_link->step();

    plan();
    listen();
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
    m_self_view.setLocation(m_motion.getProjection2D());
    m_self_view.setDirection(m_motion.getDirection2D());
    m_self_view.setFootprint(m_payload.getFootprint());

    updateAgentReport();
}


void Agent::plan(void)
{
    /* Schedule activities: */
    double tv_now = VirtualTime::now();
    bool resources_ok = true;
    // for(auto& r : m_resources) {
    //     if(r.second->getCapacity() / r.second->getMaxCapacity() < 0.25f) {
    //         resources_ok = false;
    //         break;
    //     }
    // }
    if(m_activities->pending() == 0 && m_current_activity == nullptr && resources_ok) {
        /* Ensures that old activities are removed from the agent's knowledge base: */
        m_activities->purge();

        /* Create a temporal activity (won't be added to the Activities Handler): */
        double t_end = tv_now + Config::agent_planning_window * Config::time_step;
        auto tmp_act = createActivity(tv_now, t_end);

        /* Compute and display payoff for the temporal activity object: */
        m_environment->computePayoff(tmp_act, true);

        #if 0
        /* DEBUG ================================================================================ */
        double debug_tstart = tv_now + Random::getUi(10, 100) * Config::time_step;
        double debug_tend   = debug_tstart + Random::getUi(100, 300) * Config::time_step;
        if(debug_tend > t_end) {
            debug_tend = t_end;
            if(debug_tstart > debug_tend) {
                return;
            }
        }
        auto new_act = createActivity(debug_tstart, debug_tend, m_payload.getAperture());
        m_activities->add(new_act);
        return;
        /* ====================================================================================== */
        #endif

        /* Based on previously computed payoff, generate potential activities: */
        auto act_gens = m_environment->generateActivities(tmp_act);
        std::vector<std::shared_ptr<Activity> > acts;
        for(auto& ag : act_gens) {
            if(ag.t0 < ag.t1) {
                acts.push_back(createActivity(ag.t0, ag.t1));
            } else {
                Log::warn << "[" << m_id << "] Was trying to create an activity where tstart >= tend (1). Skipping.\n";
            }
        }
        double ts = acts.front()->getStartTime();
        double te = acts.back()->getEndTime();

        /* Create scheduler instance: */
        if(!m_link_energy_available && !m_link->isEnabled()) {
            /* Only enable if it was disabled due to energy issues: */
            Log::dbg << "The link for agent " << m_id << " will now be (re-)enabled.\n";
            m_link->enable();
        }
        m_link_energy_available = true;
        m_resources.at("energy")->setReservedCapacity(0.1f /* TODO: change for Config::reserved_energy_capacity */);
        std::map<std::string, std::shared_ptr<const Resource> > rs_cpy_const(m_resources.begin(), m_resources.end());
        GAScheduler scheduler(ts, te, rs_cpy_const);

        /* Configure chromosomes: */
        std::vector<double> t0s;
        std::vector<int> steps;
        for(auto& act : acts) {
            t0s.push_back(act->getStartTime());
            double steps_exact = (act->getEndTime() - act->getStartTime()) / Config::time_step;
            int steps_int = std::round(steps_exact);
            if(steps_exact < 1.0 && steps_int < 1) {
                Log::err << "Activity duration is smaller than 1 step, but rounding did not solve this. After rounding: " << steps_int << " steps.\n";
            }
            steps.push_back(steps_int);
        }
        scheduler.setChromosomeInfo(t0s, steps, m_payload.getResourceRates());

        /* Configure activity/chromosome payoffs for fitness calculation: */
        for(unsigned int i = 0; i < act_gens.size(); i++) {
            scheduler.setAggregatedPayoff(i, act_gens[i].c_coord, act_gens[i].c_payoffs);
        }

        /* Run the scheduler: */
        auto result = scheduler.schedule();

        /* Store the result: */
        for(auto& setimes : result) {
            if(setimes.first < setimes.second) {
                auto new_act = createActivity(setimes.first, setimes.second);
                m_activities->add(new_act);
            } else {
                Log::warn << "[" << m_id << "] Was trying to create an activity where "
                    << "tstart(" << VirtualTime::toString(setimes.first) << ") >= "
                    << "tend(" << VirtualTime::toString(setimes.second) << ") (2). Skipping.\n";
            }
        }
    }
}

bool Agent::encounter(std::string aid)
{
    if((!m_activities->isCapturing() && !Config::link_allow_during_capture) || Config::link_allow_during_capture) {
        /* Accept the connection (always, with everyone). */
        return true;
    } else {
        /* There's an activity executing, will ignore this connection. */
        Log::dbg << "[" << m_id << "] Ignoring encounter with " << aid << " because a task is ongoing and simultaneity is not allowed.\n";
        return false;
    }
}

void Agent::connected(std::string aid)
{
    /* Prepare the list of activities to send. */
    m_activity_exchange_pool[aid] = m_activities->getActivitiesToExchange(aid);
}

void Agent::listen(void)
{
    auto rcv = m_link->readRxQueue();
    if(rcv.size() > 0) {
        for(auto& act : rcv) {
            std::vector<sf::Vector3f> traj_vec;
            auto traj = act->getTrajectory();
            for(auto& p : *traj) {
                traj_vec.push_back(p.second);
            }
            BasicInstrument tmp_imodel(act->getAperture(), -1.f);
            tmp_imodel.setDimensions(m_environment->getEnvModelInfo());
            auto active_cells = findActiveCells(traj->cbegin()->first, traj_vec, &tmp_imodel);
            act->setActiveCells(active_cells);
            m_activities->add(act);
        }
    }
    for(auto& aep : m_activity_exchange_pool) {
        if(aep.second.size() > 0) {
            /* Push these to the link interface, for this agent: */
            for(auto& a : aep.second) {
                m_link->scheduleSend(std::static_pointer_cast<const Activity>(a), aep.first);
            }
            aep.second.clear();
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
            m_current_activity->setActive(false);
            Log::dbg << "Agent " << m_id << " is ending activity " << m_current_activity->getId()
                << ", T = [" << VirtualTime::toString(m_current_activity->getStartTime())
                << ", " << VirtualTime::toString(m_current_activity->getEndTime()) << ").\n";
            m_payload.disable();
            for(auto& r : m_resources) {
                r.second->removeRate(m_current_activity.get());
            }
            m_current_activity = nullptr;
            if(m_link_energy_available && !m_link->isEnabled() && !Config::link_allow_during_capture) {
                /* Can be re-enabled now: */
                m_link->enable();
            }
        }
    }

    if(m_current_activity == nullptr && m_activities->isCapturing()) {
        m_current_activity = m_activities->getCurrentActivity();
        /* This activity has to start: */
        Log::dbg << "Agent " << m_id << " is starting activity " << m_current_activity->getId() << ".\n";
        m_current_activity->setActive();

        if(!Config::link_allow_during_capture) {
            /* Must disable link at this point. */
            m_link->disable();
        }

        m_payload.enable();
        for(auto& r : m_resources) {
            r.second->addRate(m_payload.getResourceRate(r.first), m_current_activity.get());
        }
    }
}

void Agent::consume(void)
{
    float consumed_link_energy = m_link->readEnergyConsumed();
    float reserved_capacity = m_resources.at("energy")->getReservedCapacity();
    if(consumed_link_energy <= reserved_capacity) {
        m_resources.at("energy")->setReservedCapacity(reserved_capacity - consumed_link_energy);
        m_resources.at("energy")->applyOnce(consumed_link_energy);
    } else {
        m_resources.at("energy")->setReservedCapacity(0.f);
        m_resources.at("energy")->applyOnce(reserved_capacity);
        Log::warn << "Agent " << m_id << " has consumed all the energy capacity reserved to links. ";
        Log::warn << "Disabling its link until next schedule cycle.\n";
        m_link->disable();
        m_link_energy_available = false;
    }

    /* Update resources: */
    for(auto& r : m_resources) {
        try {
            r.second->step();
        } catch(const std::runtime_error& e) {
            Log::err << "Resource violation exception catched. Will continue for debugging purposes.\n";
        }
    }
}

void Agent::showResources(bool d)
{
    m_display_resources = d;
}

std::vector<sf::Vector2i> Agent::getWorldFootprint(const std::vector<std::vector<sf::Vector3f> >& lut) const
{
    return m_payload.getVisibleCells(lut, true);
}

bool Agent::operator==(const Agent& ra)
{
    return (ra.getId() == getId());
}

bool Agent::operator!=(const Agent& ra)
{
    return !(*this == ra);
}

std::vector<ActivityCell> Agent::findActiveCells(
    double t0,
    const std::vector<sf::Vector3f>& ps,
    const Instrument* instrument,
    std::map<double, sf::Vector3f>* a_pos) const
{
    return findActiveCells(t0, ps.cbegin(), ps.cend(), instrument, a_pos);
}

std::vector<ActivityCell> Agent::findActiveCells(
    double t0,
    const std::vector<sf::Vector3f>::const_iterator& ps0,
    const std::vector<sf::Vector3f>::const_iterator& ps1,
    const Instrument* instrument,
    std::map<double, sf::Vector3f>* a_pos) const
{
    struct default_lut_idx {
        int v = -1;
    };
    std::vector<ActivityCell> a_cells;
    std::map<int, std::map<int, default_lut_idx> > a_cells_lut;

    /* Find active cells and their times: */
    double t = t0;
    for(auto p = ps0; p != ps1; p++) {
        sf::Vector2f p2d = AgentMotion::getProjection2D(*p, t);
        if(a_pos != nullptr) {
            a_pos->emplace(t, *p);
        }
        std::vector<sf::Vector2i> cell_coords;
        if(Config::motion_model == AgentMotionType::ORBITAL) {
            float insap = instrument->getAperture() / 2.f;
            cell_coords = instrument->getVisibleCells(
                m_environment->getPositionLUT(),            /* Look-up table. */
                instrument->getSlantRangeAt(insap, *p),     /* Distance is the slant range. */
                *p,                                         /* 3-d position in ECI frame. */
                false,                                      /* `false` = model cells. */
                t                                           /* Current time in JD. */
            );
        } else {
            /* For 2-d motion models swath actually equals to the aperture. */
            cell_coords = instrument->getVisibleCells(instrument->getAperture(), p2d, false);
        }
        for(auto& cit : cell_coords) {
            /* Check whether that cell was already in the list: */
            int idx = a_cells_lut[cit.x][cit.y].v;
            if(idx != -1) {
                if(a_cells[idx].ready) {
                    /* Was added but T1 was already set. Create a new pair T0 and T1. */
                    double* prev_t0s = a_cells[idx].t0s;
                    double* prev_t1s = a_cells[idx].t1s;
                    int new_size = a_cells[idx].nts + 1;
                    a_cells[idx].t0s = new double[new_size];
                    a_cells[idx].t1s = new double[new_size];
                    for(unsigned int i = 0; i < a_cells[idx].nts; i++) {
                        a_cells[idx].t0s[i] = prev_t0s[i];
                        a_cells[idx].t1s[i] = prev_t1s[i];
                    }
                    delete[] prev_t0s;
                    delete[] prev_t1s;
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
                cell.t0s = new double[1];
                cell.t1s = new double[1];
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
    return a_cells;
}

std::shared_ptr<Activity> Agent::createActivity(double t0, double t1)
{
    if(t0 >= t1 || t0 < VirtualTime::now()) {
        Log::err << "Agent " << m_id << " failed when creating activity, start and end times are wrong: "
            << std::fixed << std::setprecision(6) << t0 << ", " << t1 << std::defaultfloat << " ["
            << (int)(t0 >= t1) << "|" << (int)(t0 < VirtualTime::now()) << "]\n";
        throw std::runtime_error("Error creating activity (1)");
    }
    unsigned int n_steps = std::ceil((t1 - t0) / Config::time_step);
    unsigned int n_delay = (t0 - VirtualTime::now()) / Config::time_step;
    std::vector<sf::Vector3f> ps = m_motion.propagate(n_delay + n_steps);

    if(ps.size() != n_delay + n_steps) {
        Log::err << "Agent " << m_id << " failed when creating activity, unexpected propagation points ("
            << n_delay + n_steps << " req., " << ps.size() << " returned).\n";
        throw std::runtime_error("Error creating activity (2)");
    }

    std::map<double, sf::Vector3f> a_pos;
    std::vector<sf::Vector3f>::const_iterator it0 = ps.cbegin() + n_delay;
    std::vector<sf::Vector3f>::const_iterator it1 = ps.cbegin() + n_delay + n_steps;
    std::vector<ActivityCell> a_cells = findActiveCells(t0, it0, it1, &m_payload, &a_pos);
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
