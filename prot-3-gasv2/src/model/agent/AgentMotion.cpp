/***********************************************************************************************//**
 *  The motion model of an agent in the world.
 *  @class      AgentMotion
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-06
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "AgentMotion.hpp"

CREATE_LOGGER(AgentMotion)

AgentMotion::AgentMotion(Agent* aptr, sf::Vector2f init_pos, sf::Vector2f init_vel)
    : m_param_lb_speed(1.0)
    , m_agent(aptr)
    , m_world_h(Config::world_height)
    , m_world_w(Config::world_width)
{
    switch(Config::motion_model) {
        case AgentMotionType::LINEAR_BOUNCE:
            {
                if(init_pos == sf::Vector2f(-1.f, -1.f)) {
                    init_pos = sf::Vector2f(
                        Random::getUf(0.f, m_world_w),
                        Random::getUf(0.f, m_world_h)
                    );
                }
                if(init_vel == sf::Vector2f(0.f, 0.f)) {
                    float theta = Random::getUf(0.f, 360.f);
                    init_vel = {
                        std::cos(theta),
                        std::sin(theta)
                    };
                    init_vel *= Config::agent_speed;
                }
                m_position.push_back(init_pos);
                m_velocity.push_back(init_vel);
            }
            break;
        case AgentMotionType::LINEAR_INFINITE:
            Log::err << "[" << m_agent->getId() << "] Unimplemented motion model LINEAR_INFINITE.\n";
            throw std::runtime_error("Unimplemented motion model.");
            break;
        case AgentMotionType::SINUSOIDAL:
            Log::err << "[" << m_agent->getId() << "] Unimplemented motion model SINUSOIDAL.\n";
            throw std::runtime_error("Unimplemented motion model.");
            break;
        case AgentMotionType::ORBITAL:
            Log::err << "[" << m_agent->getId() << "] Unimplemented motion model ORBITAL.\n";
            throw std::runtime_error("Unimplemented motion model.");
            break;
    }
}

void AgentMotion::step(void)
{
    if(m_position.size() == 1) {
        propagate(2);
    }

    if(m_position.size() > 1) {
        m_position.erase(m_position.begin());
        m_velocity.erase(m_velocity.begin());
    } else {
        Log::warn << "[" << m_agent->getId() << "] Agent motion failure (" << m_position.size() << ").\n";
    }
}

void AgentMotion::clearPropagation(void)
{
    std::vector<sf::Vector2f> p(1, m_position.front());
    std::vector<sf::Vector2f> v(1, m_velocity.front());
    m_position.swap(p);
    m_velocity.swap(v);
}

std::vector<sf::Vector2f> AgentMotion::propagate(unsigned int nsteps)
{
    if(nsteps > m_position.size()) {
        switch(Config::motion_model) {
            case AgentMotionType::LINEAR_BOUNCE:
            {
                sf::Vector2f p0 = m_position.back();
                sf::Vector2f v0 = m_velocity.back();
                sf::Vector2f p, v;
                unsigned int count = nsteps - m_position.size();
                for(unsigned int i = 0; i < count; i++) {
                    sf::Vector2f dp = v0 * Config::time_step;
                    move(p0, v0, dp, p, v);
                    m_position.push_back(p);
                    m_velocity.push_back(v);
                    p0 = p;
                    v0 = v;
                }
            }
            break;
            default:
            /* Does nothing. */
            break;
        }
        return m_position;
    } else {
        return std::vector<sf::Vector2f>(m_position.begin(), m_position.begin() + nsteps);
    }
}

void AgentMotion::move(sf::Vector2f p0, sf::Vector2f v0, sf::Vector2f dp, sf::Vector2f& p, sf::Vector2f& v) const
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

bool AgentMotion::inBounds(const sf::Vector2f& p) const
{
    return (p.x >= 0.f && p.x <= m_world_w) && (p.y >= 0.f && p.y <= m_world_h);
}
