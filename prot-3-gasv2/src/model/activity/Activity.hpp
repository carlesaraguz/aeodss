/***********************************************************************************************//**
 *  Atomic observation task performed by a single Agent.
 *  @class      Activity
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-07
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef ACTIVITY_HPP
#define ACTIVITY_HPP

#include "prot.hpp"
#include "EnvModel.hpp"

class SegmentView;

struct ActivityCell {
    unsigned int x;     /* Cell column number in the environment model (not real world coordinates). */
    unsigned int y;     /* Cell row number in the environment model (not real world coordinates). */
    double* t0s;        /* Times when this cell starts becoming active by the activity. */
    double* t1s;        /* Times when this cell ends being active by the activity. */
    unsigned int nts;   /* The number of elements in the lists t0s and t1s. */
    bool ready;         /* The information in this cell is ready/valid. */
};

enum class ActivityPriorityModel {
    BASIC               /* Considers delta time, age and confidence extremes. */
};

class Activity
{
public:
    /*******************************************************************************************//**
     *  Constructs an activity created by agent `agent_id` without giving it an activity
     *  identifier. Both the creation time and the last-update time are set to VirtualTime::now().
     **********************************************************************************************/
    Activity(std::string agent_id, int id = -1);

    /*******************************************************************************************//**
     *  Destructs an activity, effectively releasing resources allocated to its list of active
     *  cells.
     **********************************************************************************************/
    ~Activity(void);

    /*******************************************************************************************//**
     *  Copy-constructor. Does not copy active cell list nor cell look-up table.
     **********************************************************************************************/
    Activity(const Activity& other);

    /*******************************************************************************************//**
     *  Copy-assign operator. Does not copy active cell list nor cell look-up table.
     **********************************************************************************************/
    Activity& operator=(const Activity& other);

    /*******************************************************************************************//**
     *  Auto-generated move constructor.
     **********************************************************************************************/
    Activity(Activity&& other) = default;

    /*******************************************************************************************//**
     *  Auto-generated move-assign operator.
     **********************************************************************************************/
    Activity& operator=(Activity&& other) = default;

    /*******************************************************************************************//**
     *  Computes the utility for the confidence c, normalised between 1 and `fl`. Utility is
     *  computed with the logistic function (σ(·)) in two parts:
     *      For c <= 0.5, U = σ(1-2c)
     *      For c > 0.5,  U = σ(2(c-0.5))
     *  @param  c       Confidence value; must be in the range [0,1].
     *  @param  fl      Minimum utility (between 0 and 1). This does not guarantee that the minum
     *                  obtained by the model is actually this value, but only acts as a normalising
     *                  factor that computes U'=fl+(1-fl)U. For fl=0, U'=U.
     **********************************************************************************************/
    static float utility(float c, float fl = 0.f);

    /*******************************************************************************************//**
     *  Computes the decay value for a given time `t`.
     **********************************************************************************************/
    static float decay(double t);

    /*******************************************************************************************//**
     *  Getter for the confirmed activity flag. Activities that are confirmed have confidence of 1,
     *  and are confirmed to be carried out (either in the future, present or past).
     **********************************************************************************************/
    bool isConfimed(void) const { return m_confirmed; }

    /*******************************************************************************************//**
     *  Getter for the discarded activity flag. Activities that are discarded have confidence of 0,
     *  and are confirmed to not be carried out (neither in the future, present or past).
     **********************************************************************************************/
    bool isDiscarded(void) const { return m_discarded; }

    /*******************************************************************************************//**
     *  Returns true iff the activity has been either confirmed or discarded.
     **********************************************************************************************/
    bool isFact(void) const { return (m_confirmed && !m_discarded) || (!m_confirmed && m_discarded); }

    /*******************************************************************************************//**
     *  Returns true if agent identified with `aid` is the owner of this task. False is returned
     *  otherwise.
     **********************************************************************************************/
    bool isOwner(std::string aid) const { return aid == m_agent_id; }

    /*******************************************************************************************//**
     *  Retrieves the start and end times for the cell with model coordinates x and y.
     *  @param  x       X-axis model coordinate.
     *  @param  y       Y-axis model coordinate.
     *  @param  t0s     Pointer to the array of start times (returned).
     *  @param  t1s     Pointer to the array of end times (returned).
     *  @returns        The pointer to start and end times are returned in t0s and t1s. If the cell
     *                  (x, y) can't be found, nullptr is set in t0s and t1s and the funcion returns
     *                  0. The number of items is returned otherwise.
     **********************************************************************************************/
    int getCellTimes(unsigned int x, unsigned int y, double** t0s, double** t1s) const;

    /*******************************************************************************************//**
     *  Gets the coordinates of all active cells for this task.
     **********************************************************************************************/
    std::vector<sf::Vector2i> getActiveCells(void) const;

    /*******************************************************************************************//**
     *  Gets the coordinates of active cells for this activity that are active at time t (i.e. start
     *  time <= t < end time).
     **********************************************************************************************/
    std::vector<sf::Vector2i> getActiveCells(double t) const;

    /*******************************************************************************************//**
     *  Gets the visual object that represents this activity. This object can only be retrieved if
     *  the activity is ready (i.e. if it has its trajectory and active cells set). If this function
     *  is never called, the activity object will never allocate resources for its graphical
     *  representation.
     **********************************************************************************************/
    std::shared_ptr<SegmentView> getView(std::string owner);

    /*******************************************************************************************//**
     *  Sets the active-flag of this activity. If an activity is set to active, it implies that
     *  there is an instrument capturing data. An activity that is set to active (a = true) is
     *  automatically confirmed.
     *  @note           This action updates the last-update time value.
     *  @param  a       Active flag value, true = the task is active, false = inactive.
     **********************************************************************************************/
    void setActive(bool a = true);

    /*******************************************************************************************//**
     *  Returns whether an activity is currently been executed (true) or not (false).
     **********************************************************************************************/
    bool isActive(void) const { return m_active; }

    /*******************************************************************************************//**
     *  Getter for the trajectory of this activity, as set by its creating agent.
     **********************************************************************************************/
    std::shared_ptr<std::map<double, sf::Vector3f> > getTrajectory(void) const { return m_trajectory; }
    // const std::map<double, sf::Vector3f>& getTrajectory(void) const { return m_trajectory; }

    /*******************************************************************************************//**
     *  Returns the number of points in the trajectory of this activity.
     **********************************************************************************************/
    std::size_t getPositionCount(void) const { return m_trajectory->size(); }

    /*******************************************************************************************//**
     *  Sets the trajectory and the active cells for this activity. This function is meant to be
     *  called once an activity is created (i.e. both its trajectory and active cells are known and
     *  have been computed by the creating agent).
     **********************************************************************************************/
    void setTrajectory(const std::map<double, sf::Vector3f>& pts, const std::vector<ActivityCell>& acs);

    /*******************************************************************************************//**
     *  Sets the active cells of this activity. This is meant to be called by an agent that has
     *  received this activity and is processing it to become part of its knowledge base. The
     *  activity becomes ready (i.e. activity times can be queried and graphical resources can be
     *  allocated) iff the trajectory for this activity had preiously been set.
     **********************************************************************************************/
    void setActiveCells(const std::vector<ActivityCell>& acs);

    /*******************************************************************************************//**
     *  Defines this activity to be carried out (either in the future, present or past). If c is
     *  true (default), this function also sets the value of the activity's confidence to 1 and
     *  unsets the discarded flag (i.e. isFact returns true after this). Else, this function only
     *  sets the confirmed flag to false but does not modify the discarded flag nor confidence
     *  figure.
     *  @note           This action updates the last-update time value.
     *  @note           This function is not equivalent to setDiscarded(!c).
     **********************************************************************************************/
    void setConfirmed(bool c = true);

    /*******************************************************************************************//**
     *  Confirms this activity to never be carried out (both in the future, present and past). If c
     *  is true (default), this function also sets the value of the activity's confidence to 0 and
     *  unsets the confirmed flag (i.e. isFact returns true after this). Else, this function only
     *  sets the discarded flag to false but does not modify the confirmed flag nor confidence
     *  figure.
     *  @note           This action updates the last-update time value.
     *  @note           This function is not equivalent to setConfirmed(!c).
     **********************************************************************************************/
    void setDiscarded(bool d = true);

    /*******************************************************************************************//**
     *  Updates the agent static confidence about this activity. This function should only be called
     *  by owner agents.
     *  @note           This action updates the last-update time value.
     *  @param  c       The new confidence value, in the range [0, 1].
     **********************************************************************************************/
    void setConfidenceBaseline(float c);

    /*******************************************************************************************//**
     *  Updates the confidence value from the confidence baseline to the current value, which is
     *  necessarily closer to confirmation u(a)=1. This updates the internal confidence variable
     *  that will be read by other agents with reportConfidence.
     **********************************************************************************************/
    void setConfidence(void);

    /*******************************************************************************************//**
     *  Getter for the static confidence value set by the creating agent.
     **********************************************************************************************/
    float reportConfidence(void) const { return m_confidence; }

    /*******************************************************************************************//**
     *  Getter for the static confidence value set by the creating agent.
     **********************************************************************************************/
    float getConfidenceBaseline(void) const { return m_confidence_baseline; }

    /*******************************************************************************************//**
     *  Computes the priority of this activity using the model type defined in `pmodel_type`.
     **********************************************************************************************/
    float getPriority(ActivityPriorityModel pmodel_type) const;

    /*******************************************************************************************//**
     *  Set the activity identifier (locally and univocally set by the creating agent).
     **********************************************************************************************/
    void setId(int id);

    /*******************************************************************************************//**
     *  Getter for the identifier of the creating agent.
     **********************************************************************************************/
    std::string getAgentId(void) const { return m_agent_id; }

    /*******************************************************************************************//**
     *  Getter for the activity identifier.
     **********************************************************************************************/
    unsigned int getId(void) const { return m_id; }

    /*******************************************************************************************//**
     *  Getter for the start time of this activity.
     *  @return     The time at which this activity should become active. -1 is returned if this
     *              activity is not ready and this value is unknown (i.e. because its trajectory has
     *              not been defined).
     **********************************************************************************************/
    double getStartTime(void) const;

    /*******************************************************************************************//**
     *  Getter for the end time of this activity.
     *  @return     The time at which this activity should no longer be active. -1 is returned if
     *              this activity is not ready and this value is unknown (i.e. because its
     *              trajectory has not been defined).
     **********************************************************************************************/
    double getEndTime(void) const;

    /*******************************************************************************************//**
     *  Getter for the last-update time of this activity.
     **********************************************************************************************/
    double getLastUpdateTime(void) const { return m_last_update; }

    /*******************************************************************************************//**
     *  Getter for the creation time of this activity.
     **********************************************************************************************/
    double getCreationTime(void) const { return m_creation_time; }

    /*******************************************************************************************//**
     *  Getter for the instrument aperture of the creating agent.
     **********************************************************************************************/
    float getAperture(void) const { return m_aperture; }

    /*******************************************************************************************//**
     *  Setter for the aperture of the instrument of the agent that created this activity.
     **********************************************************************************************/
    void setAperture(float ap) { m_aperture = ap; }

    /*******************************************************************************************//**
     *  Clones all relevant state values from aptr into this object. This effectively copies all
     *  member attributes except the ones that relate to trajectory and active cells.
     **********************************************************************************************/
    void clone(std::shared_ptr<Activity> aptr);

    /*******************************************************************************************//**
     *  Compares the start times of activities, iff both activities are ready.
     *  @throws     std::runtime_error if either of the two activities are not ready.
     **********************************************************************************************/
    bool operator<(const Activity& a) const;

    /*******************************************************************************************//**
     *  Determines whether this activity is equal to `a` by comparing their agent and activity
     *  identifiers as well as their last-update time.
     **********************************************************************************************/
    bool operator==(const Activity& a) const;

    /*******************************************************************************************//**
     *  Outputs a string representation of this activity details in `os`.
     **********************************************************************************************/
    friend std::ostream& operator<<(std::ostream& os, const Activity& act);

private:
    std::string m_agent_id;
    int m_id;
    bool m_confirmed;
    bool m_discarded;
    bool m_ready;
    bool m_active;
    float m_confidence;
    float m_confidence_baseline;
    float m_aperture;
    double m_last_update;
    double m_creation_time;

    /* Spatio-temporal information: */
    std::shared_ptr<SegmentView> m_self_view;
    std::shared_ptr<std::map<double, sf::Vector3f> > m_trajectory;
    std::vector<ActivityCell> m_active_cells;
    std::map<unsigned int, std::map<unsigned int, int> > m_cell_lut;
};



#endif /* ACTIVITY_HPP */
