/***********************************************************************************************//**
 *  An agent in the world.
 *  @class      Agent
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-06
 *  @version    0.2
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef AGENT_HPP
#define AGENT_HPP

#include "prot.hpp"
#include "TimeStep.hpp"
#include "AgentMotion.hpp"
#include "AgentLink.hpp"
#include "EnvModel.hpp"
#include "BasicInstrument.hpp"
#include "Resource.hpp"
#include "AgentView.hpp"
#include "HasView.hpp"

class Agent : public TimeStep, public HasView
{
public:
    Agent(std::string id);
    Agent(std::string id, sf::Vector2f init_pos, sf::Vector2f init_vel);

    /* Public member functions: */
    void step(void) override;

    /* Getters and setters: */
    std::string getId(void) const { return m_id; }
    const AgentView& getView(void) const override { return m_self_view; }
    const AgentMotion& getMotion(void) const { return m_motion; }
    std::vector<sf::Vector2i> getWorldFootprint(void) const;
    bool isCapturing(void) const { return m_payload.isEnabled(); }

    /* Agent Link: */
    std::shared_ptr<AgentLink> getLink(void) const { return m_link; }

    /* Overloaded operators: */
    bool operator==(const Agent& ra);
    bool operator!=(const Agent& ra);

private:
    /* Model parameters and components: */
    BasicInstrument m_payload;
    AgentMotion m_motion;
    std::shared_ptr<AgentLink> m_link;
    ActivityHandler m_activities;
    // std::vector<std::string> m_gs_network;

    /* State and resources: */
    EnvModel m_environment;
    std::map<std::string, std::shared_ptr<Resource> > m_resources;

    /* Views: */
    AgentView m_self_view;

    /* Other: */
    // AgentReport m_report;
    std::string m_id;
};

#endif /* AGENT_HPP */
