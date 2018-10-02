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

Agent::Agent(std::string id, sf::Vector2f init_pos, sf::Vector2f init_vel)
    : m_id(id)
    , m_self_view(id)
    , m_motion(this, init_pos, init_vel)
    , m_environment(this, (Config::world_width / Config::model_unity_size), (Config::world_height / Config::model_unity_size))
    , m_link(std::make_shared<AgentLink>(this))
    , m_activities(std::make_shared<ActivityHandler>(this))
    , m_current_activity(nullptr)
    , m_display_resources(false)
{
    m_payload.setDimensions(m_environment.getEnvModelInfo());
    m_activities->setAgentId(m_id);
    initializeResources();
}

Agent::Agent(std::string id)
    : m_id(id)
    , m_self_view(id)
    , m_motion(this)
    , m_environment(this, (Config::world_width / Config::model_unity_size), (Config::world_height / Config::model_unity_size))
    , m_link(std::make_shared<AgentLink>(this))
    , m_activities(std::make_shared<ActivityHandler>(this))
    , m_current_activity(nullptr)
    , m_display_resources(false)
{
    m_payload.setDimensions(m_environment.getEnvModelInfo());
    m_activities->setAgentId(m_id);
    initializeResources();
}

void Agent::initializeResources(void)
{
    auto energy = std::make_shared<CumulativeResource>(this, "energy", 10.f);
    m_resources["energy"] = std::static_pointer_cast<Resource>(energy);
    auto storage = std::make_shared<CumulativeResource>(this, "storage", 10.f);
    m_resources["storage"] = std::static_pointer_cast<Resource>(storage);
}

void Agent::step(void)
{
    float tv_now = VirtualTime::now();
    m_motion.step();
    m_payload.setPosition(m_motion.getPosition());

    /* Randomly schedule activities: */
    while(m_activities->pending(m_id) <= 1) {
        auto last = m_activities->getLastActivity();
        float t0, t1, t2;
        if(last == nullptr) {
            t0 = tv_now;
        } else {
            t0 = last->getEndTime();
        }
        t1 = std::max(t0 + Random::getUi(100, 200) * Config::time_step, tv_now);
        t2 = t1 + Random::getUi(50, 100) * Config::time_step;
        m_activities->add(createActivity(t1, t2, m_payload.getSwath()));
    }

    /* Execute activities: */
    auto actptr = m_activities->getCurrentActivity();
    if(m_current_activity != actptr && actptr != nullptr) {
        /* There's a new activity to start: */
        m_current_activity = actptr;
        Log::dbg << "Agent " << m_id << " is starting activity " << m_current_activity->getId() << ".\n";
        m_payload.enable();
        for(auto& r : m_resources) {
            r.second->addRate(m_payload.getResourceRate(r.first), m_current_activity.get());
        }
    } else if(actptr == nullptr && m_current_activity != nullptr) {
        /* The current activity needs to be completed/finished. */
        Log::dbg << "Agent " << m_id << " is ending activity " << m_current_activity->getId() << ".\n";
        m_payload.disable();
        for(auto& r : m_resources) {
            r.second->removeRate(m_current_activity.get());
        }
        m_current_activity = nullptr;
    }

    /* Update resources: */
    for(auto& r : m_resources) {
        r.second->step();
    }

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
    m_self_view.setPosition(m_motion.getPosition());
    m_self_view.setDirection(m_motion.getVelocity());
    m_self_view.setFootprint(m_payload.getFootprint());
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
        Log::err << "Agent " << m_id << " failed when creating activity, start and end times are wrong: " << t0 << ", " << t1 << ".\n";
        throw std::runtime_error("Error creating activity");
    }
    unsigned int n_steps = (t1 - t0) / Config::time_step;
    unsigned int n_delay = (t0 - VirtualTime::now()) / Config::time_step;
    std::vector<sf::Vector2f> ps = m_motion.propagate(n_delay + n_steps + 1);

    std::map<float, sf::Vector2f> a_pos;
    std::vector<ActivityCell> a_cells;

    struct default_lut_idx {
        int v = -1;
    };
    std::map<int, std::map<int, default_lut_idx> > a_cells_lut;

    /* Find active cells and their times: */
    float t = t0;
    for(auto p = ps.begin() + n_delay; p != ps.begin() + n_delay + n_steps; p++) {
        a_pos[t] = *p;
        auto cell_coords = m_payload.getVisibleCells(swath, *p);
        for(auto& cit : cell_coords) {
            /* Check whether that cell was already in the list: */
            int idx = a_cells_lut[cit.x][cit.y].v;
            if(idx != -1) {
                /* Previously added, update its t1 time: */
                a_cells[idx].t1 = t;
            } else {
                /* New cell, add it now: */
                ActivityCell cell;
                cell.x  = cit.x;
                cell.y  = cit.y;
                cell.t0 = t;
                cell.t1 = t;
                a_cells.push_back(cell);
                a_cells_lut[cit.x][cit.y].v = a_cells.size() - 1; /* Update look up table. */
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
