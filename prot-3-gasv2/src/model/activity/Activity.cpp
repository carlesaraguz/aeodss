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
    , m_ready(false)
    , m_self_view(nullptr)
    , m_aperture(0.f)
    , m_last_update(VirtualTime::now())
    , m_creation_time(VirtualTime::now())
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
    , m_last_update(other.m_last_update)
    , m_self_view(nullptr)
    , m_aperture(other.m_aperture)
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
    m_last_update = other.m_last_update;
    m_self_view = nullptr;
    m_aperture = other.m_aperture;
    return *this;
}

void Activity::clone(std::shared_ptr<Activity> aptr)
{
    setConfidence(aptr->getConfidence());
    setConfirmed(aptr->isConfimed());
    m_last_update = aptr->getLastUpdateTime();
    m_creation_time = aptr->getCreationTime();
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
    m_confirmed = c;
    if(c) {
        m_discarded = false;
        m_confidence = 1.f;
    }
    m_last_update = VirtualTime::now();
}

void Activity::setDiscarded(bool d)
{
    m_discarded = d;
    if(d) {
        m_confirmed = false;
        m_confidence = 0.f;
    }
    m_last_update = VirtualTime::now();
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
    m_active = a;
    if(a) {
        setConfirmed(true);
    }
    if(m_self_view != nullptr) {
        m_self_view->setActive(a);
    }
}

void Activity::setConfidence(float c)
{
    m_confidence = c;
    m_last_update = VirtualTime::now();
}

float Activity::getPriority(ActivityPriorityModel pmodel_type) const
{
    float retval = 0.f;
    switch(pmodel_type) {
        case ActivityPriorityModel::BASIC:
        {
            double t_now = VirtualTime::now();
            double a = t_now - m_creation_time;     /* Age (1).     */
            double b = t_now - m_last_update;       /* Age (2).     */
            double c = 0.0;                         /* Confidence.  */
            double d;                               /* Delta-time.  */
            if(m_confidence >= 0.8f) {
                c = (m_confidence - 0.8f) / 0.2;
            } else if(m_confidence <= 0.2f) {
                c = 1.0 - (m_confidence / 0.2);
            }
            d = getStartTime();
            if(!m_ready || d < 0.0) {
                Log::err << "Can't compute priority of " << *this << ", because it is not ready.\n";
                throw std::runtime_error("Can't compute priority of an activity that is not ready.");
            }
            d = std::fabs(d - t_now);

            /* Normalize and saturate times: */
            a /= Config::goal_target;
            b /= Config::goal_target;
            d /= Config::goal_target * 2.0;
            a = std::min(std::max(1.0 - a, 0.0), 1.0);
            b = std::min(std::max(1.0 - b, 0.0), 1.0);
            d = std::min(std::max(1.0 - d, 0.0), 1.0);

            double wa = 3.0;
            double wb = 4.0;
            double wc = 0.0;
            double wd = 3.0;
            double wsum = wa + wb + wc + wd;
            wa /= wsum;
            wb /= wsum;
            wc /= wsum;
            wd /= wsum;
            retval = (wa * a) + (wb * b) + (wc * c) + (wd * d);
        }
    }
    return retval;
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
        Log::warn << "Trying to retrieve start time of activity "
            << m_agent_id << ":" << m_id << ", but its trajectory has yet not been defined.\n";
        return -1.f;
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
        os << "S:" << VirtualTime::toString(act.getStartTime()) << " E:" << VirtualTime::toString(act.getEndTime());
    } else {
        os << "not ready";
    }
    os << "}";
    return os;
}
