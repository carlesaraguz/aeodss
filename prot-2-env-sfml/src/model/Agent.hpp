/***********************************************************************************************//**
 *  An agent in the world.
 *  @class      Agent
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-22
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef AGENT_HPP
#define AGENT_HPP

#include "prot.hpp"
#include "Intent.hpp"
#include "AgentView.hpp"
#include "EnvModel.hpp"
#include "EnvModelView.hpp"
#include "SegmentView.hpp"
#include "Random.hpp"

class Agent
{
public:
    Agent(std::string id,
        unsigned int wwidht = Config::world_width,
        unsigned int wheight = Config::world_height);
    Agent(std::string id, unsigned int wwidht, unsigned int wheight, sf::Vector2f ipos);

    sf::Vector2f step(void);
    AgentView& getAgentView(void) { return m_self_v; }
    std::map<unsigned int, SegmentView>& getSegmentViews(void) { return m_segments; }
    EnvModelView& getEnvView(void);
    sf::Vector2f getWorldSize(void) const { return sf::Vector2f(m_world_w, m_world_h); }

private:
    struct AgentState {
        sf::Vector2f position;
        sf::Vector2f velocity;
        float resource;
    };

    /* State: */
    std::map<float, AgentState> m_states;
    AgentState m_current_state;
    float m_current_time;

    /* Self model: */
    float m_speed;
    float m_swath;
    float m_range;
    std::string m_id;
    unsigned int m_predict_size;
    unsigned int m_intent_id;

    /* World model: */
    unsigned int m_world_w;
    unsigned int m_world_h;
    unsigned int m_world_model_w;
    unsigned int m_world_model_h;
    std::map<std::string, std::map<unsigned int, Intent> > m_intents;
    EnvModel m_environment;

    /* Graphical representations: */
    EnvModelView m_env_v;
    AgentView m_self_v;
    std::map<unsigned int, SegmentView> m_segments;

    void propagateState(void);
    void move(sf::Vector2f p0, sf::Vector2f v0, sf::Vector2f dp, sf::Vector2f& p, sf::Vector2f& v);
    void plan(void);
    void execute(void);
    void recomputeResource(void);
    bool isCloseToBounds(const sf::Vector2f& p) const;
    bool inBounds(const sf::Vector2f& p) const;
    void setViewText(void);
    void setViewDirection(void);
    unsigned int quadrant(sf::Vector2f v) const;
};

#endif /* AGENT_HPP */
