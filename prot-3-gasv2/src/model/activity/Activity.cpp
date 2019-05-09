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
#include "AgentMotion.hpp"

CREATE_LOGGER(Activity)

Activity::Activity(std::string agent_id, int id)
    : m_agent_id(agent_id)
    , m_id(id)
    , m_confirmed(false)
    , m_discarded(false)
    , m_active(false)
    , m_confidence(0.f)
    , m_confidence_baseline(0.f)
    , m_ready(false)
    , m_self_view(nullptr)
    , m_aperture(0.f)
    , m_last_update(VirtualTime::now())
    , m_creation_time(VirtualTime::now())
    , m_has_been_sent(false)
    , m_sending(false)
{ }

Activity::~Activity(void)
{
    for(auto& cell : m_active_cells) {
        delete[] cell.t0s;
        delete[] cell.t1s;
    }
}

Activity::Activity(const Activity& other)
    : m_agent_id(other.m_agent_id)
    , m_id(other.m_id)
    , m_confirmed(other.m_confirmed)
    , m_discarded(other.m_discarded)
    , m_trajectory(other.m_trajectory)
    /* m_active_cells is not copied. */
    , m_ready(false)
    , m_confidence(other.m_confidence)
    , m_confidence_baseline(other.m_confidence_baseline)
    , m_last_update(other.m_last_update)
    , m_self_view(nullptr)
    , m_aperture(other.m_aperture)
    , m_has_been_sent(other.m_has_been_sent)
    , m_sending(false)
{ }

Activity& Activity::operator=(const Activity& other)
{
    m_agent_id = other.m_agent_id;
    m_id = other.m_id;
    m_confirmed = other.m_confirmed;
    m_discarded = other.m_discarded;
    m_trajectory = other.m_trajectory;
    /* m_active_cells is not copied. */
    m_ready = false;
    m_confidence = other.m_confidence;
    m_confidence_baseline = other.m_confidence_baseline;
    m_last_update = other.m_last_update;
    m_self_view = nullptr;
    m_aperture = other.m_aperture;
    m_has_been_sent = other.m_has_been_sent;
    m_sending = false;
    return *this;
}

void Activity::clone(std::shared_ptr<Activity> aptr)
{
    m_confidence = aptr->m_confidence;
    m_confidence_baseline = aptr->m_confidence_baseline;
    m_confirmed = aptr->m_confirmed;
    m_discarded = aptr->m_discarded;
    m_creation_time = aptr->m_creation_time;
    m_last_update = aptr->m_last_update;
    m_has_been_sent = aptr->m_has_been_sent;
    m_sending = false;
    /*  NOTE: the following members are not modified/cloned:
     *  - Agent ID & activity ID: these are supposed to be equal or we don't care.
     *  - Trajectory: we're only updating state. Trajectory is expected to remain the same.
     *  - Active cells: idem than above.
     *  - Segment view: trajectory has not changed, so the view needs not change either.
     *  - Aperture: this is not a state value but a static parameter.
     *  - Ready-flag: only affects view and trajectory, which haven't changed.
     **/
}

void Activity::setConfirmed(bool c)
{
    if(!isFact()) {
        m_confirmed = c;
        if(c) {
            m_discarded = false;
            m_confidence = 1.f;
            m_confidence_baseline = 1.f;
        }
        m_last_update = VirtualTime::now();
        Log::dbg << "Activity [" << m_agent_id << ":" << m_id << "] has been confirmed.\n";
    } else if(isConfimed()) {
        Log::warn << "Trying to change fact [" << m_agent_id << ":" << m_id << "] is confirmed; setting confirmed to \'"
            << std::boolalpha << c << "\'. This call has no effect.\n";
    } else {
        Log::err << "Trying to change fact [" << m_agent_id << ":" << m_id << "] is discarded; setting confirmed to \'"
            << std::boolalpha << c << "\'. This call has no effect.\n";
    }
}

void Activity::setDiscarded(bool d)
{
    if(!isFact()) {
        m_discarded = d;
        if(d) {
            m_confirmed = false;
            m_confidence = 0.f;
            m_confidence_baseline = 0.f;
        }
        m_last_update = VirtualTime::now();
        Log::err << "Activity [" << m_agent_id << ":" << m_id << "] has been discarded.\n";
    } else if(isDiscarded()) {
        Log::warn << "Trying to change fact [" << m_agent_id << ":" << m_id << "] is discarded; setting discarded to \'"
            << std::boolalpha << d << "\'. This call has no effect.\n";
    } else {
        Log::err << "Trying to change fact [" << m_agent_id << ":" << m_id << "] is confirmed; setting discarded to \'"
            << std::boolalpha << d << "\'. This call has no effect.\n";
    }
}

std::vector<sf::Vector2i> Activity::getActiveCells(void) const
{
    std::vector<sf::Vector2i> retval;
    for(auto& ac : m_active_cells) {
        retval.push_back(sf::Vector2i(ac.x, ac.y));
    }
    return retval;
}

std::vector<sf::Vector2i> Activity::getActiveCells(double t) const
{
    std::vector<sf::Vector2i> retval;
    for(auto& ac : m_active_cells) {
        for(unsigned int i = 0; i < ac.nts; i++) {
            if(ac.t0s[i] <= t && ac.t1s[i] > t) {
                retval.push_back(sf::Vector2i(ac.x, ac.y));
            }
        }
    }
    return retval;
}

int Activity::getCellTimes(unsigned int x, unsigned int y, double** t0s, double** t1s) const
{
    if(t0s == nullptr || t1s == nullptr) {
        Log::err << "Error getting cell times for activity " << *this << ". Null pointer (" << (void*)t0s << ", " << (void*)t1s << ").\n";
        return 0;
    }
    if(m_cell_lut.find(x) != m_cell_lut.end()) {
        if(m_cell_lut.at(x).find(y) != m_cell_lut.at(x).end()) {
            auto idx = m_cell_lut.at(x).at(y);
            *t0s = m_active_cells[idx].t0s;
            *t1s = m_active_cells[idx].t1s;
            return m_active_cells[idx].nts;
        }
    }
    /* Not found: */
    *t0s = nullptr;
    *t1s = nullptr;
    return 0;
}

void Activity::setTrajectory(const std::map<double, sf::Vector3f>& pts, const std::vector<ActivityCell>& acs)
{
    m_trajectory = std::make_shared<std::map<double, sf::Vector3f> >(pts.begin(), pts.end());  /* Copies trajectory.   */
    m_active_cells = acs;   /* Copies active cells. */
    unsigned int it = 0;
    for(auto& ac : m_active_cells) {
        m_cell_lut[ac.x][ac.y] = it++;
    }
    m_ready = true;
}

void Activity::setActiveCells(const std::vector<ActivityCell>& acs)
{
    m_active_cells = acs;   /* Copies active cells. */
    m_cell_lut.clear();
    unsigned int it = 0;
    for(auto& ac : m_active_cells) {
        m_cell_lut[ac.x][ac.y] = it++;
    }
    m_ready = m_trajectory->size() > 0;
}

void Activity::setId(int id)
{
    if(id >= 0) {
        m_id = id;
    }
}

void Activity::setActive(bool a)
{
    if(!isFact() && a) {
        setConfirmed(true);
    }
    m_active = a;
    if(m_self_view != nullptr) {
        m_self_view->setActive(a);
    }
}

void Activity::setConfidence(void)
{
    if(!isFact()) {
        double m_last_update = VirtualTime::now();
        double t_ref = getStartTime() - (Config::activity_confirm_window * Config::time_step);
        double delta = t_ref - m_last_update;
        if(delta < 0.0) {
            /* This activity should have been confirmed, but it's not. */
            Log::err << "Activity [" << m_agent_id << ":" << m_id << "] has not been confirmed and time has exceeded "
                << "the confirmation window. This is unexpected.\n";
            delta = 0.0;
        }
        m_confidence = std::pow(m_confidence_baseline, (1.f - std::pow(delta, Config::confidence_mod_exp)));
    } /* ... else is a fact and we cannot make changes. */
}

void Activity::setConfidenceBaseline(float c)
{
    m_confidence_baseline = c;
    m_confidence = c;
    m_last_update = VirtualTime::now();
}

float Activity::getPriority(ActivityPriorityModel pmodel_type) const
{
    float retval = 0.f;
    switch(pmodel_type) {
        case ActivityPriorityModel::BASIC:
        {
            float d = decay(m_last_update); /* Decay: time since last update. */
            float u = utility(m_confidence, Config::utility_floor);  /* Confidence-utility.  */
            retval = (Config::decay_weight * d) + (Config::utility_weight * u);
            /* decay_weight + utility_weight should be = 1. */
        }
    }
    return retval;
}

float Activity::decay(double /* t */)
{
    return 0.f;
}

float Activity::utility(float c, float fl)
{
    float utility, mod_c;
    if(c <= 0.5f) {
        mod_c = 1.f - 2.f * c;
    } else {
        mod_c = 2.f * (c - 0.5f);
    }
    utility = 1.f/(1.f + std::exp(-Config::utility_k * (mod_c - 0.5f)));
    return fl + (1.f - fl) * utility;
}

std::shared_ptr<SegmentView> Activity::getView(std::string owner)
{
    if(m_self_view == nullptr && m_ready) {
        std::vector<sf::Vector2f> vec_pos;
        vec_pos.reserve(m_trajectory->size());
        for(auto& p : *m_trajectory) {
            vec_pos.push_back(AgentMotion::getProjection2D(p.second, p.first));
        }
        m_self_view = std::make_shared<SegmentView>(vec_pos, m_agent_id + ":" + std::to_string(m_id));
        if(m_agent_id == owner) {
            m_self_view->setOwnership(true);
        } else {
            m_self_view->setOwnership(false);
        }
        m_self_view->setActive(m_active);
    }
    return m_self_view;
}

double Activity::getStartTime(void) const
{
    if(m_trajectory->size() > 0 && m_ready) {
        return m_trajectory->cbegin()->first;
    } else {
        Log::warn << "Trying to retrieve start time of activity "
            << m_agent_id << ":" << m_id << ", but its trajectory has yet not been defined.\n";
        return -1.f;
    }
}

double Activity::getEndTime(void) const
{
    if(m_trajectory->size() > 0 && m_ready) {
        return m_trajectory->crbegin()->first;
    } else {
        Log::warn << "Trying to retrieve end time of activity "
            << m_agent_id << ":" << m_id << ", but its trajectory has yet not been defined.\n";
        return -1.f;
    }
}

void Activity::setStartTime(double t)
{
    if(m_trajectory->size() > 1 && m_ready) {
        auto i = m_trajectory->cbegin();
        auto j = std::next(i);
        if(t < j->first) {
            if(std::abs(t - i->first) > Config::time_step) {
                Log::warn << "Changing start time of an activity will result in a change of more than one time step.\n";
            }
            auto p = i->second;
            m_trajectory->erase(i);         /* Remove the previous start point. */
            m_trajectory->emplace(t, p);    /* Add it again with the new time. */
        } else {
            Log::err << "Can't change the start time of an activity for a value that is past the second trajectory point.\n";
        }
    } else {
        Log::warn << "Trying to set start time of activity "
            << m_agent_id << ":" << m_id << ", but its trajectory has yet not been defined.\n";
    }
}

void Activity::setEndTime(double t)
{
    if(m_trajectory->size() > 1 && m_ready) {
        auto i = m_trajectory->crbegin();
        auto j = std::prev(i);
        if(t > j->first) {
            if(std::abs(t - i->first) > Config::time_step) {
                Log::warn << "Changing end time of an activity will result in a change of more than one time step.\n";
            }
            auto p = i->second;
            m_trajectory->erase(std::prev(m_trajectory->end()));    /* Remove the previous end point. */
            m_trajectory->emplace(t, p);    /* Add it again with the new time. */
        } else {
            Log::err << "Can't change the end time of an activity for a value that is before the previous-to-last trajectory point.\n";
        }
    } else {
        Log::warn << "Trying to set end time of activity "
            << m_agent_id << ":" << m_id << ", but its trajectory has yet not been defined.\n";
    }
}

bool Activity::operator<(const Activity& a) const
{
    if(a.m_ready && m_ready) {
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
    return (m_id == a.m_id) && (m_agent_id == a.m_agent_id) && (m_last_update == a.m_last_update);
}

std::ostream& operator<<(std::ostream& os, const Activity& act)
{
    os << "{Activity " << act.m_agent_id << ":" << act.m_id << "; ";
    if(act.m_trajectory != nullptr) {
        os << "traj:" << act.m_trajectory->size() << " points, ";
    } else {
        os << "traj: undefined, ";
    }
    os << "actc:" << act.m_active_cells.size() << " cells, ";
    if(act.m_ready) {
        os << std::fixed << std::setprecision(2);
        os << "S:" << VirtualTime::toString(act.getStartTime(), true, true) << " ";
        os << "E:" << VirtualTime::toString(act.getEndTime(), true, true) << ", ";
    } else {
        os << "not ready, ";
    }
    if(act.isConfimed()) {
        os << "Fact, Confirmed}";
    } else if(act.isDiscarded()) {
        os << "Fact, Discarded}";
    } else {
        os << "BC: " << act.m_confidence_baseline << " C: " << act.m_confidence << "}";
    }
    return os;
}
