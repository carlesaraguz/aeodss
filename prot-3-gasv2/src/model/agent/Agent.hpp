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
#include "hashers.hpp"
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
#include "VirtualTime.hpp"

#include "GAScheduler.hpp"

class AgentBuilder;

class Agent : public TimeStep, public HasView, public ReportGenerator
{
public:
    std::string m_dbg_str;  /* DEBUG TODO REMOVE */

    Agent(std::string id);
    Agent(std::string id, sf::Vector2f init_pos, sf::Vector2f init_vel);
    Agent(AgentBuilder* ab);

    /* Public member functions: */
    void step(void) override;
    void stepParallel(void);
    void stepSequential(void);
    void plan(void);
    void updatePosition(void);
    void showResources(bool d = true);

    /* Getters and setters: */
    std::string getId(void) const { return m_id; }
    const AgentView& getView(void) const override { return m_self_view; }
    std::shared_ptr<EnvModel> getEnvironment(void) { return m_environment; }
    const AgentMotion& getMotion(void) const { return m_motion; }
    std::shared_ptr<const ActivityHandler> getActivityHandler(void) const { return m_activities; }
    std::vector<sf::Vector2i> getWorldFootprint(const std::vector<std::vector<sf::Vector3f> >& lut) const;
    bool isCapturing(void) const { return m_payload.isEnabled(); }

    /* Agent Link: */
    std::shared_ptr<AgentLink> getLink(void) const { return m_link; }

    /* Helpers: */
    void displayActivities(ActivityDisplayType af);

    /* Overloaded operators: */
    bool operator==(const Agent& ra);
    bool operator!=(const Agent& ra);

private:
    bool m_print_resources;
    /* Model parameters and components: */
    BasicInstrument m_payload;
    AgentMotion m_motion;
    std::shared_ptr<AgentLink> m_link;
    bool m_link_energy_available;
    std::shared_ptr<ActivityHandler> m_activities;
    std::shared_ptr<Activity> m_current_activity;
    std::map<std::string, std::vector<std::shared_ptr<Activity> > > m_activity_exchange_pool;

    /* State and resources: */
    std::shared_ptr<EnvModel> m_environment;
    std::map<std::string, std::shared_ptr<Resource> > m_resources;

    /* Other: */
    AgentView m_self_view;
    std::string m_id;
    bool m_display_resources;
    double m_replan_horizon;

    std::vector<ActivityCell> findActiveCells(double t0, double t1,
        const std::vector<sf::Vector3f>& ps,
        const Instrument* instrument, std::map<double, sf::Vector3f>* a_pos = nullptr) const;
    std::vector<ActivityCell> findActiveCells(double t0, double t1,
        const std::vector<sf::Vector3f>::const_iterator& ps0, const std::vector<sf::Vector3f>::const_iterator& ps1,
        const Instrument* instrument, std::map<double, sf::Vector3f>* a_pos = nullptr) const;
    std::shared_ptr<Activity> createActivity(double t0, double t1);
    void initializeResources(void);

    void listen(void);
    void execute(void);
    void consume(void);
    bool encounter(std::string aid);
    void connected(std::string aid);

    void configAgentReport(void);
    void updateAgentReport(void);

};

#endif /* AGENT_HPP */
