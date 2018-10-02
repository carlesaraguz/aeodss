/***********************************************************************************************//**
 *  The motion model of an agent in the world.
 *  @class      AgentMotion
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-06
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef AGENT_MOTION_HPP
#define AGENT_MOTION_HPP

#include "prot.hpp"
#include "TimeStep.hpp"
#include "Random.hpp"

class Agent;

class AgentMotion : public TimeStep
{
public:
    AgentMotion(Agent* aptr, sf::Vector2f init_pos = {-1.f, -1.f}, sf::Vector2f init_vel = {0.f, 0.f});

    void step(void) override;
    std::vector<sf::Vector2f> propagate(unsigned int nsteps);
    void clearPropagation(void);

    /* Getters and setters: */
    sf::Vector2f getPosition(void) const { return m_position.front(); }
    sf::Vector2f getVelocity(void) const { return m_velocity.front(); }

private:
    std::vector<sf::Vector2f> m_position;
    std::vector<sf::Vector2f> m_velocity;
    float m_param_lb_speed;

    float m_world_h;
    float m_world_w;
    Agent* m_agent;

    void move(sf::Vector2f p0, sf::Vector2f v0, sf::Vector2f dp, sf::Vector2f& p, sf::Vector2f& v) const;
    bool inBounds(const sf::Vector2f& p) const;
};

#include "Agent.hpp"

#endif /* AGENT_MOTION_HPP */
