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
    , m_predict_size(Random::getUf(1, Config::agent_propagation_size))
    , m_intent_id(0)
    , m_environment(wwidht / Config::model_unity_size, wheight / Config::model_unity_size, wwidht, wheight)
    , m_intent_handler(id)
{
    std::cout << "Agent: " << m_id << ". Predict size: " << m_predict_size << "\n";

    float theta = Random::getUf(0.f, 360.f);
    m_current_state.position = ipos;
    m_current_state.velocity = {
        m_speed * std::cos(theta),
        m_speed * std::sin(theta)
    };
    m_current_state.resource = 0.f;
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
    float t, r0, r;
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
        if(r0 + Config::capacity_restore >= Config::max_capacity) {
            r = Config::max_capacity;
        } else {
            r = r0 + Config::capacity_restore;
        }
        m_states[t] = {p, v, r};
    }
}

sf::Vector2f Agent::step(void)
{
    /* Update state and generate a new one: */
    if(m_states.size() > 0) {
        m_current_time  = m_states.begin()->first;
        m_current_state = m_states.begin()->second;
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
    /*  Bounces:
     *  bx0 -> bounce at x=0.
     *  bx1 -> bounce at x=w.
     *  by0 -> bounce at y=0.
     *  by1 -> bounce at y=h.
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

void Agent::plan(void)
{
    if(m_intent_handler.getIntentCount(m_id, m_current_time) >= 3) {
        return;
    }
    /* The agent is allowed to generate more intents: */
    float last_intent_time = m_intent_handler.getLastIntentTime(m_id);

    /* Unoptimized search: */
    bool creating_intent = false, i_continue = false;
    Intent i(m_swath);
    float i_duration;
    auto s = m_states.find(last_intent_time);
    if(s == m_states.end()) {
        s = m_states.begin();
    }
    float start_resource = 0.f;
    for(; s != m_states.end(); s++) {
        if(m_intent_handler.getIntentCount(m_id, m_current_time) >= 3) {
            break;
        }
        if(s->first <= last_intent_time) {
            continue;
        }
        if(!creating_intent) {
            if(    s->second.resource >= Config::max_capacity * 0.7f
                && !isCloseToBounds(s->second.position)
                && (s != std::prev(m_states.end()))
            ) {
                /* Start the creation of a new intent: */
                creating_intent = true;
                i_continue = true;
                start_resource = s->second.resource;
                i.tstart = s->first;
                i.pstart = s->second.position;
            }
        } else {
            i_duration = s->first - i.tstart;
            /* Check resource state: */
            i_continue &= (start_resource - i_duration * (Config::capacity_consume - Config::capacity_restore) > Config::max_capacity * 0.1f);
            /* Check proximity to borders: */
            i_continue &= !isCloseToBounds(s->second.position);
            /* Check prediction extent: */
            i_continue &= (s != std::prev(m_states.end()));

            if(!i_continue) {
                if(i_duration >= 60.f) {
                    i.tend = s->first;
                    i.pend = s->second.position;
                    i.id   = m_intent_id++;
                    i.setAgentId(m_id);
                    last_intent_time = i.tend;
                    m_intent_handler.createIntent(i);
                    recomputeResource();
                }
                creating_intent = false;
            }
        }
    } /* end for loop. */
}

void Agent::execute(void)
{
    if(m_intent_handler.isActiveAt(m_current_time)) {
        m_environment.setValueByWorldCoord(
            m_current_time,
            m_current_state.position.x,
            m_current_state.position.y,
            255.f,
            m_swath / 2.f
        );
    }
}

void Agent::recomputeResource(void)
{
    float r = m_states[m_current_time].resource;
    int count = 0;
    auto s = m_states.find(m_current_time);
    if(s == m_states.end()) {
        std::cerr << "[Agent " << m_id << "] Something went wrong: current state was not found in predictions.\n";
        std::exit(-1);
    }
    for(; s != m_states.end(); s++) {
        auto active_intents = m_intent_handler.getActiveIntentsAt(s->first, m_id);
        if(active_intents) count++;
        // std::cout << "[Agent " << m_id << "] (" << std::setw(5) << count << ")" << std::fixed << std::setprecision(4)
        //     << " T = " << s->first << ","
        //     << " R = " << r << " + " << Config::capacity_restore << " - " << active_intents * Config::capacity_consume
        //     << " = " << r + Config::capacity_restore - (active_intents * Config::capacity_consume) << "\n";
        r += Config::capacity_restore - active_intents * Config::capacity_consume;
        s->second.resource = r;
        if(s->second.resource < 0.f) {
            std::cerr << "[Agent " << m_id << "] Something went wrong, negative resource capacity (" <<
                std::fixed << std::setprecision(3) << s->second.resource << ") at t=" << s->first;
            std::cerr << " (" << count << " steps since t0 = " << m_current_time << "). Active intents: " << active_intents;
            std::cerr << std::endl;
            std::exit(-1);
        }
    }
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
        std::cout << "Agent [" << m_id << "] disconnected from [" << aid << "]\n";

    } else if(foundf != m_link_table[false].end()) {
        auto aptrf = m_link_table[false][aid];
        m_link_table[false].erase(foundf);
        m_link_table[true][aptrf->getId()] = aptrf;
        m_has_communicated[aptrf->getId()] = false;
        std::cout << "Agent [" << m_id << "] connected to [" << aid << "]\n";

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
                IntentHandler::Opts ihopts;
                ihopts.filter = IntentSelection::ALL;
                auto pkt_received = a.second->exchangeIntents(m_intent_handler.getIntents(ihopts));
                m_intent_handler.processRcvIntents(pkt_received);
            }
        }
    }
}

/** TODO
 *  - If I create a new intent, then I probably have to set the m_new_intents to true.
 *  - Clean intents that are not mine/have not been executed.
 */


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
