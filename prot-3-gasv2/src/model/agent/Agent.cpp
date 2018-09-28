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
    , m_activities(this)
    , m_environment(this, (Config::world_width / Config::model_unity_size), (Config::world_height / Config::model_unity_size))
    , m_link(std::make_shared<AgentLink>(this))
{
    m_payload.setDimensions(m_environment.getEnvModelInfo());
}

Agent::Agent(std::string id)
    : m_id(id)
    , m_self_view(id)
    , m_motion(this)
    , m_activities(this)
    , m_environment(this, (Config::world_width / Config::model_unity_size), (Config::world_height / Config::model_unity_size))
    , m_link(std::make_shared<AgentLink>(this))
{
    m_payload.setDimensions(m_environment.getEnvModelInfo());
}

void Agent::step(void)
{
    m_motion.step();
    m_payload.setPosition(m_motion.getPosition());
    m_self_view.setPosition(m_motion.getPosition());
    m_self_view.setDirection(m_motion.getVelocity());
    m_self_view.setFootprint(m_payload.getFootprint());

    while(m_activities.count(m_id) < 3) {
        auto last = m_activities.getLastActivity();
        float t0;
        if(last == nullptr) {
            t0 = VirtualTime::now();
        } else {
            /* t0 = last-> TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO */
        }
    }
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
