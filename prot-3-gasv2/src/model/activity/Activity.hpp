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
    float t0;           /* Time when this cell starts becoming active by the activity. */
    float t1;           /* Time when this cell ends being active by the activity. */
};

class Activity
{
public:
    Activity(std::string agent_id, int id = -1);
    Activity(const Activity& other);
    Activity& operator=(const Activity& other);
    Activity(Activity&& other);
    Activity& operator=(Activity&& other);

    /* State: */
    bool isConfimed(void) const { return m_confirmed; }
    bool isDiscarded(void) const { return m_discarded; }
    bool isFact(void) const { return (m_confirmed && !m_discarded) || (!m_confirmed && m_discarded); }
    bool isOwner(std::string aid) const { return aid == m_agent_id; }
    void getCellTimes(unsigned int x, unsigned int y, float& t0, float& t1) const;
    std::vector<sf::Vector2i> getActiveCells(void) const;
    std::shared_ptr<SegmentView> getView(void);

    /* Positions: */
    std::size_t getPositionCount(void) const { return m_trajectory.size(); }
    void setTrajectory(const std::map<float, sf::Vector2f>& pts, const std::vector<ActivityCell>& acs);

    /* Payoff: */
    float computePayoff(const EnvModel& e);

    /* Getters & setters: */
    void setConfirmed(bool c = true);
    void setDiscarded(bool d = true);
    void setId(int id);
    std::string getAgentId(void) const { return m_agent_id; }
    unsigned int getId(void) const { return m_id; }
    float getStartTime(void) const;
    float getEndTime(void) const;

    /* Operator overloads: */
    bool operator<(const Activity& a) const;
    bool operator==(const Activity& a) const;

    /* Friend debug functions: */
    friend std::ostream& operator<<(std::ostream& os, const Activity& act);

private:
    std::string m_agent_id;
    int m_id;
    float m_payoff;
    bool m_confirmed;
    bool m_discarded;
    bool m_ready;
    float m_confidence;

    /* Spatio-temporal information: */
    std::shared_ptr<SegmentView> m_self_view;
    std::map<float, sf::Vector2f> m_trajectory;
    std::vector<ActivityCell> m_active_cells;
    std::map<unsigned int, std::map<unsigned int, int> > m_cell_lut;
};



#endif /* ACTIVITY_HPP */
