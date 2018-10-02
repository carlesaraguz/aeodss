/***********************************************************************************************//**
 *  An agent in the world.
 *  @class      Agent
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-22
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Agent.hpp"


Agent::Agent(std::string id, unsigned int wwidht, unsigned int wheight)
    : Agent(id, wwidht, wheight, sf::Vector2f(Random::getUf(0.f, wwidht), Random::getUf(0.f, wheight)))
{ }

Agent::Agent(std::string id, unsigned int wwidht, unsigned int wheight, sf::Vector2f ipos)
    : m_swath(Random::getUf(Config::agent_swath_max, Config::agent_swath_min))
    , m_range(Random::getUf(Config::agent_range_max, Config::agent_range_min))
    , m_speed(Config::agent_speed)
    , m_id(id)
    , m_self_v(id)
    , m_world_w(wwidht)
    , m_world_h(wheight)
    , m_current_time(0.f)
    , m_predict_size(Config::agent_propagation_size)
    , m_intent_id(0)
    , m_environment(wwidht / Config::model_unity_size, wheight / Config::model_unity_size, wwidht, wheight)
    , m_intent_handler(id)
    , m_new_insights(true)
{
    std::clog << "Agent: " << m_id << ". Predict size: " << m_predict_size << "\n";

    float theta = Random::getUf(0.f, 360.f);
    m_current_state.position = ipos;
    m_current_state.velocity = {
        m_speed * std::cos(theta),
        m_speed * std::sin(theta)
    };
    m_current_state.resource = Config::max_capacity / 2.f;
    m_states[m_current_time] = m_current_state;

    m_self_v.setSwath(m_swath);
    m_self_v.setCommsRange(m_range);
    m_self_v.setPosition(m_current_state.position);

    setViewText();
    setViewDirection();

    m_environment.displayInView(0);
    m_environment.setLayerFunction(0, [this](EnvModel::Cell& c) {
        if(c.time == -1.f) {
            return;
        }
        float dt = m_current_time - c.time;
        if(dt <= 0.f) {
            c.value = 1.f;
        } else if(dt > Config::max_revisit_time) {
            c.value = 0.f;
        } else {
            c.value = 1.f - (dt / Config::max_revisit_time);
        }
    });

    propagateState();
}

void Agent::propagateState(void)
{
    float t, r0;
    float dt = Config::time_step;
    sf::Vector2f dp;
    sf::Vector2f p0, p, v0, v;
    while(m_states.size() < m_predict_size) {
        if(m_states.size() == 0) {
            std::cerr << "Agent " << m_id << " can't propagate its state without previous values.\n";
            break;
        }
        p0 = m_states.rbegin()->second.position;
        v0 = m_states.rbegin()->second.velocity;
        r0 = m_states.rbegin()->second.resource;
        t  = m_states.rbegin()->first + dt;
        dp = v0 * dt;
        move(p0, v0, dp, p, v);
        // if(r0 + Config::capacity_restore >= Config::max_capacity) {
        //     r = Config::max_capacity;
        // } else {
        //     r = r0 + Config::capacity_restore;
        // }
        m_states[t] = {p, v, r0};
    }
}

sf::Vector2f Agent::step(void)
{
    /* Update state and generate a new one: */
    if(m_states.size() > 0) {
        m_current_time  = m_states.begin()->first;
        m_current_state.position = m_states.begin()->second.position;
        m_current_state.velocity = m_states.begin()->second.velocity;
    }
    m_self_v.setPosition(m_current_state.position);
    setViewDirection();


    plan();
    execute();
    m_environment.updateAll();

    setViewText();

    m_states.erase(m_states.begin());
    propagateState();
    return m_current_state.position;
}

void Agent::setViewText(void)
{
    std::stringstream ss;
    ss << m_id << "\n" << std::fixed << std::setprecision(1) << m_current_state.resource;
    m_self_v.setText(ss.str());
}

void Agent::move(sf::Vector2f p0, sf::Vector2f v0, sf::Vector2f dp, sf::Vector2f& p, sf::Vector2f& v)
{
    /*  Bounce flags:
     *  bx0 -> bounces at x=0.
     *  bx1 -> bounces at x=w.
     *  by0 -> bounces at y=0.
     *  by1 -> bounces at y=h.
     **/
    bool bx0 = false, bx1 = false, by0 = false, by1 = false;
    float newx = 0.f, newy = 0.f, displacement_ratio;

    if(inBounds(p0 + dp)) {
        p = p0 + dp;
        v = v0;
    } else {
        /* Determine bounces: */
        sf::Vector2f ptmp = p0 + dp;
        if(ptmp.x < 0.f) {
            bx0 = true;
        }
        if(ptmp.x > m_world_w) {
            bx1 = true;
        }
        if(ptmp.y < 0.f) {
            by0 = true;
        }
        if(ptmp.y > m_world_h) {
            by1 = true;
        }
        if(bx0 && by0) {
            if(-ptmp.x >= -ptmp.y) {
                by0 = false;
            } else {
                bx0 = false;
            }
        } else if(by0 && bx1) {
            if(-ptmp.y >= ptmp.x - m_world_w) {
                bx1 = false;
            } else {
                by0 = false;
            }
        } else if(bx1 && by1) {
            if(ptmp.x - m_world_w >= ptmp.y - m_world_h) {
                by1 = false;
            } else {
                bx1 = false;
            }
        } else if(by1 && bx0) {
            if(ptmp.y - m_world_h >= -ptmp.x) {
                bx0 = false;
            } else {
                by1 = false;
            }
        }
        if(bx0) {
            /* Bounce at x=0: */
            newx = 0.f;
            displacement_ratio = std::fabs(p0.x / dp.x);
            newy = p0.y + displacement_ratio * dp.y;
            dp.x = -dp.x - p0.x;
            dp.y *= (1.f - displacement_ratio);
            v0.x = -v0.x;
        } else if(by0) {
            /* Bounce at y=0: */
            newy = 0.f;
            displacement_ratio = std::fabs(p0.y / dp.y);
            newx = p0.x + displacement_ratio * dp.x;
            dp.y = -dp.y - p0.y;
            dp.x *= (1.f - displacement_ratio);
            v0.y = -v0.y;
        } else if(bx1) {
            /* Bounce at x=w: */
            newx = m_world_w;
            displacement_ratio = std::fabs((m_world_w - p0.x) / dp.x);
            newy = p0.y + displacement_ratio * dp.y;
            dp.x = -dp.x - (m_world_w - p0.x);
            dp.y *= (1.f - displacement_ratio);
            v0.x = -v0.x;
        } else if(by1) {
            /* Bounce at y=h: */
            newy = m_world_h;
            displacement_ratio = std::fabs((m_world_h - p0.y) / dp.y);
            newx = p0.x + displacement_ratio * dp.x;
            dp.y = -dp.y - (m_world_h - p0.y);
            dp.x *= (1.f - displacement_ratio);
            v0.y = -v0.y;
        }
        p0.x = newx;
        p0.y = newy;
        move(p0, v0, dp, p, v);
    }
}
void Agent::computeRewardAt(EnvModel& e, float t, unsigned int tidx, const AgentState& s,
    std::vector<std::shared_ptr<GASReward> >& rptrs,
    std::vector<std::vector<std::size_t> >& rptrs_lut,
    std::map<unsigned int, std::size_t>& rptrs_cell_lut
) const
{
    for(auto& tuple : m_intent_handler.getActivePositions(t)) {
        /*  Find whether some Agents are affecting my area of interest (i.e. parts of my trajectory)
         *  at time t. If they are, the future environment model `e` will be updated.
         *  Then compute reward.
         **/
        sf::Vector2f p = std::get<0>(tuple);
        float r = std::get<1>(tuple) / 2.f;
        bool bflag = false;
        for(auto& state : m_states) {
            sf::Vector2f dist_vec = p - state.second.position;
            float dist = std::sqrt(dist_vec.x * dist_vec.x + dist_vec.y * dist_vec.y);
            if(dist <= (m_swath / 2.f) + r) {
                bflag = true;
                break; /* We don't care if it affects other points in the trajectory. */
            }
        }
        if(bflag) {
            /* Updates the propagated model: */
            e.setValueByWorldCoord(t, p.x, p.y, 1.f, r, 0);
        }
    }
    /*  Reward is the collection of cell-rewards (i.e. GASReward) obtained at the current position.
     *  Cell-rewards are computed based on the environment update function set to `e`. First, cell-
     *  reward values are computed by updating the environment. Then these values are retreived and
     *  the corresponding GASReward object is created and pushed to `rptrs`.
     **/
    e.updateAll();

    auto cell_values = e.getValuesByWorldCoord(
        s.position.x,       /* Position of agent at T=t.    */
        s.position.y,       /* Position of agent at T=t.    */
        m_swath / 2.f,      /* Cover distance.              */
        0                   /* Layer 0.                     */
    );

    unsigned int cid;
    float cval;
    std::vector<std::size_t> rs_idxs;
    for(auto& c : cell_values) {
        cid  = std::get<0>(c);
        cval = std::get<1>(c);
        if(rptrs_cell_lut.count(cid) == 0) {
            /* This cell didn't previously have a GASReward object. Create one: */
            auto r_new = std::make_shared<GASReward>();
            r_new->setValue(tidx, cval);
            rptrs.push_back(r_new);
            rptrs_cell_lut[cid] = rptrs.size() - 1;
        } else {
            /* This cell's GASReward object had been previously created: */
            auto r_old = rptrs[rptrs_cell_lut[cid]];
            r_old->setValue(tidx, cval);
        }
        rs_idxs.push_back(rptrs_cell_lut[cid]);
    }
    rptrs_lut.push_back(rs_idxs);
}

void Agent::plan(void)
{
    /* Find how many pending intents still have to be executed: */
    IntentHandler::Opts options{
        IntentSelection::OWN | IntentSelection::FUTURE,
        -1,
        m_current_time,
        m_id
    };
    int pending = 0;
    auto itable = m_intent_handler.getIntents(options);
    if(itable.find(m_id) != itable.end()) {
        pending = itable[m_id].size();
    }
    pending += m_intent_handler.getActiveIntentsAt(m_current_time, m_id);

    /* If new insights are ready and we can schedule more tasks, invoke scheduler: */
    if(pending == 0 && m_current_state.resource > 0.f) {
        std::clog << "Agent [" << m_id << "] is computing its rewards... \n";
        /* Copy the current state of the environment and find how is it going to be at T = horizon. */
        float s_time = 0.f;
        EnvModel env_cpy = m_environment;
        env_cpy.setLayerFunction(0, [&s_time](EnvModel::Cell& c) {
            float dt;
            if(c.time < 0.f) {
                dt = Config::max_revisit_time;
            } else {
                dt = s_time - c.time;
            }
            if(dt <= 0.f) {
                c.value = 0.f;
            } else if(dt > Config::max_revisit_time) {
                c.value = 10.f;
            } else {
                float a, b, k, r, l1 = 1.f, l2 = 10.f;
                if(dt <= Config::target_revisit_time) {
                    a = 0.f;
                    b = Config::target_revisit_time;
                    k = 3.f;
                    r = (a - b) / k;
                    c.value = l1 * (1 - std::exp((a - dt) / r)) / (1 - std::exp((a - b) / r));
                } else {
                    a = Config::target_revisit_time;
                    b = Config::max_revisit_time;
                    k = -10.f;
                    r = (a - b) / k;
                    c.value = (l2 - l1) * (1 - std::exp((a - dt) / r)) / (1 - std::exp((a - b) / r)) + l1;
                }
            }
        });

        /* Compute rewards and times look-up table: */
        std::vector<std::shared_ptr<GASReward> > rptrs;
        std::vector<std::vector<std::size_t> > rptrs_lut;
        std::map<unsigned int, std::size_t> rptrs_cell_lut;
        std::vector<float> times_lut;
        times_lut.reserve(m_states.size());
        int t_idx = 0;
        for(const auto& s : m_states) {
            s_time = s.first;
            times_lut.push_back(s.first);
            computeRewardAt(env_cpy, s.first, t_idx++, s.second, rptrs, rptrs_lut, rptrs_cell_lut);
        }

        m_sched.setSchedulingWindow(m_states.size() - 1, times_lut);
        m_sched.setRewards(rptrs, rptrs_lut, Aggregate::MEAN_VALUE, (m_swath / 2.f));
        m_sched.initPopulation();

        Utils::tic();
        std::clog << "Agent [" << m_id << "] Running GA Scheduler...\n";
        auto intents = m_sched.schedule(true);
        std::clog << "Agent [" << m_id << "] Activites scheduled after " << Utils::toc() << " seconds.\n";

        for(auto& i : intents) {
            /* Adjust times to the ones in the predicted vector of agent states: */
            i.setAgentId(m_id);
            i.setAgentSwath(m_swath);
            auto s0 = m_states.find(i.tstart);
            auto s1 = m_states.find(i.tend);
            if(i.tend >= Config::time_step && s0 != m_states.end() && s1 != m_states.end() && std::distance(s0, s1) > 0 && i.setPositions(s0, s1)) {
                i.id = m_intent_id++;
                m_intent_handler.createIntent(i);
                std::clog << "Agent [" << m_id << "] New capture activity [" << i.id << "]: T{" << i.tstart << ", " << i.tend << "}::" << (i.tend - i.tstart) << "\n";
            } else {
                std::cerr << "Agent [" << m_id << "] scheduled a task with an error. Discarding it:\n";
                std::clog << i;
            }
        }
        m_new_insights = false;
    }
}

void Agent::execute(void)
{
    /* Agents' actions: */
    m_intent_handler.setTime(m_current_time);
    m_current_state.resource -= m_intent_handler.getCurrentConsumption();
    if(m_current_state.resource + Config::capacity_restore >= Config::max_capacity) {
        m_current_state.resource = Config::max_capacity;
    } else {
        m_current_state.resource += Config::capacity_restore;
    }
    if(m_current_state.resource < 0.f) {
        std::cerr << "Agent [" << m_id << "] has depleted its recources completely: " << m_current_state.resource << "\n";
        std::exit(-1);
    }

    for(auto& tuple : m_intent_handler.getActivePositions()) {
        m_environment.setValueByWorldCoord(
            m_current_time,
            std::get<0>(tuple).x,
            std::get<0>(tuple).y,
            255.f,
            std::get<1>(tuple) / 2.f
        );
    }
    m_intent_handler.disposeIntents(m_current_time);
}

bool Agent::isCloseToBounds(const sf::Vector2f& p) const
{
    float d = m_swath / 2.f;
    float bx = m_world_w;
    float by = m_world_h;
    return (p.x <= d || p.x >= bx - d || p.y <= d || p.y >= by - d);
}

bool Agent::inBounds(const sf::Vector2f& p) const
{
    return (p.x >= 0.f && p.x <= m_world_w) && (p.y >= 0.f && p.y <= m_world_h);
}

void Agent::setViewDirection(void)
{
    sf::Vector2f v = m_current_state.velocity / std::sqrt(
        m_current_state.velocity.x * m_current_state.velocity.x +
        m_current_state.velocity.y * m_current_state.velocity.y
    );
    float dir = 0.f;
    switch(quadrant(v)) {
        case 1:
        case 2:
            dir = std::acos(v.x);
            break;
        case 3:
        case 4:
            dir = -std::acos(v.x);
            break;
    }
    dir *= 180.f / Config::pi;
    m_self_v.setDirection(dir);
}

unsigned int Agent::quadrant(sf::Vector2f v) const
{
    if(v.x >= 0 && v.y >= 0) {
        return 1;
    } else if(v.x <= 0 && v.y >= 0) {
        return 2;
    } else if(v.x <= 0 && v.y <= 0) {
        return 3;
    } else if(v.x >= 0 && v.y <= 0) {
        return 4;
    } else {
        return 0;
    }
}

void Agent::addAgentLink(std::shared_ptr<Agent> a, bool in_view)
{
    if(a->getId() != m_id) {
        m_link_table[in_view][a->getId()] = a;
        m_has_communicated[a->getId()] = false;
    }
}

void Agent::addAgentLink(std::map<bool, std::vector<std::shared_ptr<Agent> > > als)
{
    for(auto& a : als[true]) {
        if(a->getId() != m_id) {
            m_link_table[true][a->getId()] = a;
            m_has_communicated[a->getId()] = false;
        }
    }
    for(auto& a : als[false]) {
        if(a->getId() != m_id) {
            m_link_table[false][a->getId()] = a;
            m_has_communicated[a->getId()] = false;
        }
    }
}

void Agent::toggleAgentLink(std::string aid)
{
    auto foundt = m_link_table[true].find(aid);
    auto foundf = m_link_table[false].find(aid);

    if(foundt != m_link_table[true].end()) {
        auto aptrt = m_link_table[true][aid];
        m_link_table[true].erase(foundt);
        m_link_table[false][aptrt->getId()] = aptrt;
        std::clog << "Agent [" << m_id << "] disconnected from [" << aid << "]\n";

    } else if(foundf != m_link_table[false].end()) {
        auto aptrf = m_link_table[false][aid];
        m_link_table[false].erase(foundf);
        m_link_table[true][aptrf->getId()] = aptrf;
        m_has_communicated[aptrf->getId()] = false;
        std::clog << "Agent [" << m_id << "] connected to [" << aid << "]\n";

    } else {
        std::cerr << "[" << m_id << "] Toggling an agent link state for an unknown agent \'" << aid << "\'.\n";
    }
}

bool Agent::connect(std::string agent_requester)
{
    m_has_communicated[agent_requester] = true;
    return true;
}

IntentHandler::IntentTable Agent::exchangeIntents(IntentHandler::IntentTable pkt)
{
    m_intent_handler.processRcvIntents(pkt);
    auto pkt_to_send = m_intent_handler.getIntents(IntentHandler::select_all, &pkt);
    return pkt_to_send;
}

void Agent::doCommunicate(void)
{
    for(auto& a : m_link_table[true]) {
        if(!m_has_communicated[a.first]) {
            /* Does communicate with a: */
            if(a.second->connect(m_id)) {
                m_has_communicated[a.first] = true;
                IntentHandler::Opts ihopts;
                ihopts.filter = IntentSelection::ALL;
                auto pkt_received = a.second->exchangeIntents(m_intent_handler.getIntents(ihopts));
                m_new_insights |= m_intent_handler.processRcvIntents(pkt_received);
            }
        }
    }
}

bool Agent::isVisible(std::shared_ptr<Agent> a) const
{
    sf::Vector2f v = a->getPosition();
    v -= m_current_state.position;
    float dist = std::sqrt(v.x * v.x + v.y * v.y);
    return dist <= std::min(m_range, a->getRange());
}

bool Agent::operator==(const Agent& ra)
{
    return (ra.getId() == getId());
}

bool Agent::operator!=(const Agent& ra)
{
    return !(*this == ra);
}
