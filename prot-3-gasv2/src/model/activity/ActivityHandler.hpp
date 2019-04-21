/***********************************************************************************************//**
 *  Container and manager of activities known or generated by a single Agent.
 *  @class      ActivityHandler
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-26
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef ACTIVITY_HANDLER_HPP
#define ACTIVITY_HANDLER_HPP

#include "prot.hpp"
#include "Activity.hpp"
#include "ActivityHandlerView.hpp"

class ActivityHandler : public HasView, public ReportGenerator
{
public:
    /*******************************************************************************************//**
     *  Constructs an activity handler for the agent pointed to by aptr.
     **********************************************************************************************/
    ActivityHandler(Agent* aptr);

    /*******************************************************************************************//**
     *  Updates the confirmation status of owned activities and internal state of this handler. This
     *  function checks what is the current activity of the agent and stores information to
     *  indirectly access it faster.
     **********************************************************************************************/
    void update(void);

    /*******************************************************************************************//**
     *  Determines whether there is an activity being executed by the agent.
     **********************************************************************************************/
    bool isCapturing(void);

    /*******************************************************************************************//**
     *  Returns the next activity of this agent (i.e. that which has a start time in the future,
     *  w.r.t. `t`) or nullptr if there is none.
     *  @param  t   The time to consider activities in the future. If not provided, the function
     *              will assume t = VirtualTime::now().
     **********************************************************************************************/
    std::shared_ptr<Activity> getNextActivity(double t = -1.0) const;

    /*******************************************************************************************//**
     *  Returns the current activity of the agent if there is one. This function always returns a
     *  valid pointer iff isCapturing returns true. Otherwise, nullptr is returned.
     **********************************************************************************************/
    std::shared_ptr<Activity> getCurrentActivity(void) const;

    /*******************************************************************************************//**
     *  Get the last activity scheduled by this agent. This function always returns a valid pointer
     *  iff the return of ActivityHandler::pending is greater than 0. Otherwise nullptr is returned.
     **********************************************************************************************/
    std::shared_ptr<Activity> getLastActivity(void) const;

    /*******************************************************************************************//**
     *  Adds a new activity to the knowledge base.
     **********************************************************************************************/
    void add(std::shared_ptr<Activity> pa);

    /*******************************************************************************************//**
     *  Counts the number of known activities for an agent identified with `aid`.
     *  @param  aid     Agent identifier.
     **********************************************************************************************/
    unsigned int count(std::string aid) const;

    /*******************************************************************************************//**
     *  Gives the number of activities for the owning agent such that they end in the future. This
     *  effectively counts future activites as well as the on-going one.
     **********************************************************************************************/
    unsigned int pending(void) const;

    /*******************************************************************************************//**
     *  Sets the Agent identifier for this handler. It propagates this information to the view.
     *  @param  aid     Agent ID.
     **********************************************************************************************/
    void setAgentId(std::string aid);

    /*******************************************************************************************//**
     *  Creates an activity that is onwed by this agent. This function does not add the new activity
     *  to the internal list, but only generates the object and assigns its trajectory.
     *  @param  a_pos   A table with the trajectory for this activity where the index (double) is
     *                  the virtual time and the item (sf::Vector3f) is the propagated position of
     *                  the agent.
     **********************************************************************************************/
    std::shared_ptr<Activity> createOwnedActivity(
        const std::map<double, sf::Vector3f>& a_pos,
        const std::vector<ActivityCell>& a_cells
    );

    /*******************************************************************************************//**
     *  Finds a list of activities that could be worth sharing with agent `aid`.
     **********************************************************************************************/
    std::vector<std::shared_ptr<Activity> > getActivitiesToExchange(std::string aid);

    /*******************************************************************************************//**
     *  Setter for the instrument aperture of the owner agent.
     **********************************************************************************************/
    void setInstrumentAperture(float ap) { m_aperture = ap; }

    /*******************************************************************************************//**
     *  Sets the pointer to the environment object of the agent that owns the activity handler.
     **********************************************************************************************/
    void setEnvironment(std::shared_ptr<EnvModel> eptr) { m_env_model_ptr = eptr; }

    /*******************************************************************************************//**
     *  Implements the HasView interface.
     **********************************************************************************************/
    const sf::Drawable& getView(void) const override;

    /*******************************************************************************************//**
     *  Configure whether the view should automatically be updated as new activities are added.
     **********************************************************************************************/
    void autoUpdateView(bool auto_update = true);

    /*******************************************************************************************//**
     *  Wrapper to ActivityHandlerView::display.
     **********************************************************************************************/
    void displayInView(ActivityDisplayType adt, std::vector<std::pair<std::string, unsigned int> > filter = { });

    /*******************************************************************************************//**
     *  Erases activities that are older than the maximum revisit time (because they will never
     *  contribute to payoffs in the future.)
     **********************************************************************************************/
    void purge(void);

private:
    int m_current_activity_idx;
    std::vector<std::shared_ptr<Activity> > m_activities_own;   /* Sorted by start time. */
    std::map<std::string, std::map<unsigned int, std::shared_ptr<Activity> > > m_activities_others;
    std::string m_agent_id;
    Agent* m_agent;
    bool m_update_view;
    ActivityHandlerView m_self_view;
    unsigned int m_activity_count;
    float m_aperture;
    std::shared_ptr<EnvModel> m_env_model_ptr;

    void report(void);
};

#endif /* ACTIVITY_HANDLER_HPP */
