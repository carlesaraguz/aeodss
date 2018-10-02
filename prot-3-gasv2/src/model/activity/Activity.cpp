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
#include "SegmentView.hpp"

CREATE_LOGGER(Activity)

Activity::Activity(std::string agent_id, int id)
    : m_agent_id(agent_id)
    , m_id(id)
    , m_payoff(-1.f)
    , m_confirmed(false)
    , m_discarded(false)
    , m_confidence(0.f)
    , m_ready(false)
    , m_self_view(nullptr)
{ }

Activity::Activity(const Activity& other)
    : m_agent_id(other.m_agent_id)
    , m_id(other.m_id)
    , m_payoff(-1.f)
    , m_confirmed(other.m_confirmed)
    , m_discarded(other.m_discarded)
    // , m_trajectory(other.m_trajectory)
    // , m_active_cells(other.m_active_cells)
    , m_confidence(0.f)
    , m_ready(false)
    , m_self_view(nullptr)
{ }

Activity& Activity::operator=(const Activity& other)
{
    m_id = other.m_id;
    m_agent_id = other.m_agent_id;
    m_confirmed = other.m_confirmed;
    m_discarded = other.m_discarded;
    // m_trajectory = other.m_trajectory;
    // m_active_cells = other.m_active_cells;
    m_ready = false;
    m_payoff = -1.f;
    m_confidence = 0.f;
    m_self_view = nullptr;
    return *this;
}

Activity::Activity(Activity&& other)
    : m_agent_id(other.m_agent_id)
    , m_id(other.m_id)
    , m_payoff(-1.f)
    , m_confirmed(other.m_confirmed)
    , m_discarded(other.m_discarded)
    , m_confidence(0.f)
    , m_ready(false)
    , m_self_view(nullptr)
{ }

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

std::vector<sf::Vector2i> Activity::getActiveCells(void) const
{
    std::vector<sf::Vector2i> retval;
    for(auto& ac : m_active_cells) {
        retval.push_back(sf::Vector2i(ac.x, ac.y));
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

void Activity::setTrajectory(const std::map<float, sf::Vector2f>& pts, const std::vector<ActivityCell>& acs)
{
    m_trajectory   = pts;   /* Copies trajectory.   */
    m_active_cells = acs;   /* Copies active cells. */
    unsigned int it = 0;
    for(auto& ac : m_active_cells) {
        m_cell_lut[ac.x][ac.y] = it++;
    }
    m_ready = true;
}

void Activity::setId(int id)
{
    if(id >= 0) {
        m_id = id;
    }
}

std::shared_ptr<SegmentView> Activity::getView(void)
{
    if(m_self_view == nullptr && m_ready) {
        std::vector<sf::Vector2f> vec_pos;
        vec_pos.reserve(m_trajectory.size());
        for(auto& p : m_trajectory) {
            vec_pos.push_back(p.second);
        }
        m_self_view = std::make_shared<SegmentView>(vec_pos, m_agent_id + ":" + std::to_string(m_id));
    }
    return m_self_view;
}

float Activity::getStartTime(void) const
{
    if(m_trajectory.size() > 0 && m_ready) {
        return m_trajectory.cbegin()->first;
    } else {
        Log::warn << "Trying to retrieve start time of activity "
            << m_agent_id << ":" << m_id << ", but its trajectory has yet not been defined.\n";
        return -1.f;
    }
}

float Activity::getEndTime(void) const
{
    if(m_trajectory.size() > 0 && m_ready) {
        return m_trajectory.crbegin()->first;
    } else {
        Log::warn << "Trying to retrieve start time of activity "
            << m_agent_id << ":" << m_id << ", but its trajectory has yet not been defined.\n";
        return -1.f;
    }
}

bool Activity::operator<(const Activity& a) const
{
    if(a.m_ready) {
        return getStartTime() < a.getStartTime();
    } else {
        Log::err << "Trying to compare (<) activities failed because they are not ready.\n";
        throw std::runtime_error("Can't compare activities that are not ready.");
    }
}

bool Activity::operator==(const Activity& a) const
{
    if(a.m_id <= -1 || m_id <= -1) {
        Log::warn << "Trying to compare (==) activities may be unreliable because their ID's are not set.\n";
    }
    return (m_id == a.m_id) && (m_agent_id == a.m_agent_id);
}

std::ostream& operator<<(std::ostream& os, const Activity& act)
{
    os << "{Activity " << act.m_agent_id << ":" << act.m_id << "; ";
    if(act.m_ready) {
        os << std::fixed << std::setprecision(2);
        os << "S:" << act.getStartTime() << " E:" << act.getEndTime() << ", ";
        os << "traj:" << act.m_trajectory.size() << " points";
    } else {
        os << "not ready";
    }
    os << "}";
    return os;
}
