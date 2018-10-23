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
#include "HasView.hpp"

#include "AgentMotion.hpp"
#include "AgentLink.hpp"
#include "AgentView.hpp"

#include "EnvModel.hpp"
#include "ActivityHandler.hpp"
#include "BasicInstrument.hpp"
#include "Resource.hpp"
#include "CumulativeResource.hpp"

#include "GAScheduler.hpp"

class Agent : public TimeStep, public HasView
{
public:
    Agent(std::string id);
    Agent(std::string id, sf::Vector2f init_pos, sf::Vector2f init_vel);

    /* Public member functions: */
    void step(void) override;
    void showResources(bool d = true);

    /* Getters and setters: */
    std::string getId(void) const { return m_id; }
    const AgentView& getView(void) const override { return m_self_view; }
    std::shared_ptr<EnvModel> getEnvironment(void) { return m_environment; }
    const AgentMotion& getMotion(void) const { return m_motion; }
    std::shared_ptr<const ActivityHandler> getActivityHandler(void) const { return m_activities; }
    std::vector<sf::Vector2i> getWorldFootprint(void) const;
    bool isCapturing(void) const { return m_payload.isEnabled(); }

    /* Agent Link: */
    std::shared_ptr<AgentLink> getLink(void) const { return m_link; }

    /* Helpers: */
    void displayActivities(ActivityDisplayType af);

    /* Overloaded operators: */
    bool operator==(const Agent& ra);
    bool operator!=(const Agent& ra);

private:
    /* Model parameters and components: */
    BasicInstrument m_payload;
    AgentMotion m_motion;
    std::shared_ptr<AgentLink> m_link;
    std::shared_ptr<ActivityHandler> m_activities;
    std::shared_ptr<Activity> m_current_activity;

    /* State and resources: */
    std::shared_ptr<EnvModel> m_environment;
    std::map<std::string, std::shared_ptr<Resource> > m_resources;

    /* Other: */
    AgentView m_self_view;
    std::string m_id;
    bool m_display_resources;

    std::shared_ptr<Activity> createActivity(float t0, float t1, float swath);
    void initializeResources(void);

    void plan(void);
    void execute(void);
    void consume(void);
};

#endif /* AGENT_HPP */
