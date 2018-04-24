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
#include "IntentHandler.hpp"
#include "AgentView.hpp"
#include "EnvModel.hpp"
#include "Random.hpp"
#include "GAScheduler.hpp"

struct AgentState {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float resource;
};

class Agent
{
    typedef std::map<bool, std::map<std::string, std::shared_ptr<Agent> > > LinkTable;

public:
    Agent(std::string id,
        unsigned int wwidht = Config::world_width,
        unsigned int wheight = Config::world_height);
    Agent(std::string id, unsigned int wwidht, unsigned int wheight, sf::Vector2f ipos);

    /* Public member functions: */
    sf::Vector2f step(void);
    void addAgentLink(std::shared_ptr<Agent> a, bool in_view = false);
    void addAgentLink(std::map<bool, std::vector<std::shared_ptr<Agent> > > als);
    void toggleAgentLink(std::string aid);
    bool isVisible(std::shared_ptr<Agent> a) const;
    bool connect(std::string agent_requester);
    IntentHandler::IntentTable exchangeIntents(IntentHandler::IntentTable pkt);
    void doCommunicate(void);

    /* Getters and setters: */
    sf::Vector2f getPosition(void) const { return m_current_state.position; }
    sf::Vector2f getWorldSize(void) const { return sf::Vector2f(m_world_w, m_world_h); }
    const AgentView& getAgentView(void) const { return m_self_v; }
    const IntentHandler::SegmentTable& getSegmentViews(void) const { return m_intent_handler.getViews(); }
    EnvModelView& getEnvView(void) { return m_environment.getView(); }
    std::string getId(void) const { return m_id; }
    LinkTable& getLinks(void) { return m_link_table; }
    float getRange(void) const { return m_range; }

    /* Overloaded operators: */
    bool operator==(const Agent& ra);
    bool operator!=(const Agent& ra);

private:
    /* State: */
    std::map<float, AgentState> m_states;
    AgentState m_current_state;
    float m_current_time;
    LinkTable m_link_table;
    std::map<std::string, bool> m_has_communicated;
    IntentHandler m_intent_handler;
    unsigned int m_intent_id;
    bool m_new_insights;

    /* Self model: */
    float m_speed;
    float m_swath;
    float m_range;
    std::string m_id;
    unsigned int m_predict_size;
    GAScheduler m_sched;

    /* World model: */
    unsigned int m_world_w;
    unsigned int m_world_h;
    unsigned int m_world_model_w;
    unsigned int m_world_model_h;
    EnvModel m_environment;

    /* Graphical representations: */
    AgentView m_self_v;

    void propagateState(void);
    void move(sf::Vector2f p0, sf::Vector2f v0, sf::Vector2f dp, sf::Vector2f& p, sf::Vector2f& v);
    void plan(void);
    std::vector<std::tuple<unsigned int, float> > computeRewardAt(EnvModel& e, float t, const AgentState& s) const;
    void execute(void);
    void recomputeResource(void);
    bool isCloseToBounds(const sf::Vector2f& p) const;
    bool inBounds(const sf::Vector2f& p) const;
    void setViewText(void);
    void setViewDirection(void);
    unsigned int quadrant(sf::Vector2f v) const;
};

#endif /* AGENT_HPP */
