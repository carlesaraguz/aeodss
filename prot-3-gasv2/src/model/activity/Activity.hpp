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

struct ActivityCell {
    unsigned int x;     /* Cell column number in the environment model (not real world coordinates). */
    unsigned int y;     /* Cell row number in the environment model (not real world coordinates). */
    float t0;           /* Time when this cell starts becoming active by the activity. */
    float t1;           /* Time when this cell ends being active by the activity. */
};

class Activity
{
public:
    Activity(unsigned int id, std::string agent_id);
    Activity(const Activity& other);
    Activity& operator=(const Activity& other);
    Activity(Activity&& other);
    Activity& operator=(Activity&& other);

    /* State: */
    bool isConfimed(void) const { return m_confirmed; }
    bool isDiscarded(void) const { return m_discarded; }
    bool isFact(void) const { return (m_confirmed && !m_discarded) || (!m_confirmed && m_discarded); }
    void setConfirmed(bool c = true);
    void setDiscarded(bool d = true);
    void getCellTimes(unsigned int x, unsigned int y, float& t0, float& t1) const;
    std::vector<std::pair<unsigned int, unsigned int> > getActiveCells(void) const;

    /* Positions: */
    std::size_t getPositionCount(void) const { return m_trajectory.size(); }

    /* Payoff: */
    float computePayoff(const EnvModel& e);

    /* Getters & setters: */
    std::string getAgentId(void) const { return m_agent_id; }
    unsigned int getId(void) const { return m_id; }
    float getStartTime(void) const { return m_trajectory.cbegin().first; }
    float getEndTime(void) const { return m_trajectory.crbegin().first; }

private:
    std::string m_agent_id;
    unsigned int m_id;
    float m_payoff;
    bool m_confirmed;
    bool m_discarded;
    float m_confidence;

    /* Spatio-temporal information: */
    std::map<float, sf::Vector2f> m_trajectory;
    std::vector<ActivityCell> m_active_cells;
    std::map<unsigned int, std::map<unsigned int, int> > m_cell_lut;
};

#endif /* ACTIVITY_HPP */
