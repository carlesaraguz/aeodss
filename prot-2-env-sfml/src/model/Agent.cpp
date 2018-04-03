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
    , m_env_v(wwidht / Config::model_unity_size, wheight / Config::model_unity_size, sf::Color::Black)
    , m_world_w(wwidht)
    , m_world_h(wheight)
    , m_current_time(0.f)
    , m_predict_size(Random::getUf(1, Config::agent_propagation_size))
    , m_intent_id(0)
    , m_environment(wwidht / Config::model_unity_size, wheight / Config::model_unity_size, wwidht, wheight)

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

    setViewText();

    propagateState();
    m_states.erase(m_states.begin());
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
    if(m_intents[m_id].size() >= 3) {
        return;
    }
    /* The agent is allowed to generate more intents: */
    float last_intent_time = 0.f;
    if(m_intents[m_id].size() > 0) {
        last_intent_time = m_intents[m_id].rbegin()->second.tend;
    }
    /* Unoptimized search: */
    bool creating_intent = false, i_continue = false;
    Intent i;
    float i_duration;
    for(auto s = m_states.begin(); s != m_states.end(); s++) {
        if(m_intents[m_id].size() >= 3) {
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
                i.tstart = s->first;
                i.pstart = s->second.position;
            }
        } else {
            i_duration = s->first - i.tstart;
            /* Check resource state: */
            i_continue &= (s->second.resource - i_duration * Config::capacity_consume > Config::max_capacity * 0.1f);
            /* Check proximity to borders: */
            i_continue &= !isCloseToBounds(s->second.position);
            /* Check duration: */
            i_continue &= (i_duration < 150.f);
            /* Check prediction extent: */
            i_continue &= (s != std::prev(m_states.end()));

            if(!i_continue) {
                if(i_duration >= 60.f) {
                    i.tend = s->first;
                    i.pend = s->second.position;
                    i.id   = m_intent_id++;
                    i.setAgentId(m_id);
                    last_intent_time = i.tend;
                    m_intents[m_id].emplace(std::make_pair(i.id, i));
                    SegmentView segment(i, m_swath);
                    m_segments.emplace(std::make_pair(i.id, segment));
                    // std::cout << "[Agent " << m_id << "] Creating intent " << i.id <<
                    //     " -> T[ " << i.tstart << "···" << i.tend << " ](" << (i.tend - i.tstart) << ")";
                    // std::cout << " - R0=" << m_states.find(i.tstart)->second.resource << ", C(-)=" <<
                    //     i_duration * Config::capacity_consume << " C(+)=" << i_duration * Config::capacity_restore <<
                    //     " ===> " << i_duration * (Config::capacity_restore - Config::capacity_consume) + m_states.find(i.tstart)->second.resource << "\n";
                    recomputeResource();
                }
                creating_intent = false;
            }
        }
    } /* end for loop. */
}

void Agent::execute(void)
{
    bool capture = false;
    std::vector<unsigned int> old_intents;
    for(auto& i : m_intents[m_id]) {
        if(m_current_time >= i.second.tstart && m_current_time <= i.second.tend) {
            m_segments.at(i.second.id).setActive(true);
            capture = true;
        } else if(m_current_time > i.second.tend) {
            old_intents.push_back(i.second.id);
        }
    }
    for(auto& i : old_intents) {
        m_intents[m_id].erase(m_intents[m_id].find(i));
        m_segments.erase(m_segments.find(i));
    }
    if(capture) {
        if(m_id == "A0") {
            m_environment.setValue(m_current_state.position.x, m_current_state.position.y, 255.f);
        }
    }
}

EnvModelView& Agent::getEnvView(void)
{
    m_env_v.display(m_environment); /* Updates the view. */
    return m_env_v;
}

void Agent::recomputeResource(void)
{
    float r = m_states[m_current_time].resource;
    int count = 0;
    for(auto s = m_states.find(m_current_time); s != m_states.end(); s++) {
        int active_intents = 0;
        for(auto& i : m_intents[m_id]) {
            if(s->first >= i.second.tstart && s->first <= i.second.tend) {
                active_intents++;
            }
        }
        r += Config::capacity_restore - active_intents * Config::capacity_consume;
        s->second.resource = r;
        if(s->second.resource < 0.f) {
            std::cerr << "[Agent " << m_id << "] Something went wrong, negative resource capacity (" <<
                std::fixed << std::setprecision(3) << s->second.resource << ") at t=" << s->first;
            std::cerr << "(t0 = " << m_current_time << ") [" << count << "]";
            std::cerr << std::endl;
            std::exit(-1);
        }
        count++;
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
