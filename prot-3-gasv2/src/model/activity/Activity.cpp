/***********************************************************************************************//**
 *  Atomic observation task performed by a single Agent.
 *  @class      Activity
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-07
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Activity.hpp"

CREATE_LOGGER(Activity)

Activity::Activity(unsigned int id, std::string agent_id)
    : m_agent_id(agent_id)
    , m_id(id)
    , m_payoff(-1.f)
    , m_confirmed(false)
    , m_discarded(false)
    , m_confidence(0.f)
{ }

Activity::Activity(const Activity& other)
    : m_agent_id(other.m_agent_id)
    , m_id(other.m_id)
    , m_payoff(-1.f)
    , m_confirmed(other.m_confirmed)
    , m_discarded(other.m_discarded)
    , m_trajectory(other.m_trajectory)
    , m_active_cells(other.m_active_cells)
    , m_confidence(0.f)
{ }

Activity& Activity::operator=(const Activity& other)
{
    m_id = other.m_id;
    m_agent_id = other.m_agent_id;
    m_confirmed = other.m_confirmed;
    m_discarded = other.m_discarded;
    m_trajectory = other.m_trajectory;
    m_active_cells = other.m_active_cells;
    m_payoff = -1.f;
    m_confidence = 0.f;
    return *this;
}

Activity::Activity(Activity&& other)
    : m_agent_id(other.m_agent_id)
    , m_id(other.m_id)
    , m_payoff(-1.f)
    , m_confirmed(other.m_confirmed)
    , m_discarded(other.m_discarded)
    , m_confidence(0.f)
{
    m_trajectory = std::move(other.m_trajectory);
    m_active_cells = std::move(other.m_active_cells);
}

Activity& Activity::operator=(Activity&& other)
{
    m_id = other.m_id;
    m_agent_id = other.m_agent_id;
    m_confirmed = other.m_confirmed;
    m_discarded = other.m_discarded;
    m_trajectory = std::move(other.m_trajectory);
    m_active_cells = std::move(other.m_active_cells);
    m_payoff = -1.f;
    m_confidence = 0.f;
    return *this;
}

void Activity::setConfirmed(bool c)
{
    m_discarded = !c;
    m_confirmed = c;
}
void Activity::setDiscarded(bool d)
{
    m_discarded = d;
    m_confirmed = !d;
}

float Activity::computePayoff(const EnvModel& /* e */)
{
    Log::err << "[" << m_agent_id << ":" << m_id << "] Payoff function unimplemented.\n";
    return -1.f;
}

std::vector<std::pair<unsigned int, unsigned int> > Activity::getActiveCells(void) const
{
    std::vector<std::pair<unsigned int, unsigned int> > retval;
    for(auto& ac : m_active_cells) {
        retval.push_back({ac.x, ac.y});
    }
    return retval;
}

void Activity::getCellTimes(unsigned int x, unsigned int y, float& t0, float& t1) const
{
    if(m_cell_lut.find(x) != m_cell_lut.end()) {
        if(m_cell_lut.at(x).find(y) != m_cell_lut.at(x).end()) {
            auto idx = m_cell_lut.at(x).at(y);
            t0 = m_active_cells[idx].t0;
            t1 = m_active_cells[idx].t1;
            return;
        }
    }
    t0 = -1.f;
    t1 = -1.f;
}
