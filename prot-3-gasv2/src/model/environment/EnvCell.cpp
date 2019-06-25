/***********************************************************************************************//**
 *  A single spatial segment of the environment.
 *  @class      EnvCell
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-10
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "EnvCell.hpp"
#include "Activity.hpp"
#include "Agent.hpp"

CREATE_LOGGER(EnvCell)

EnvCell::EnvCell(Agent* agnt, unsigned int cx, unsigned int cy)
    : x(cx)
    , y(cy)
    , m_agent(agnt)
{ }

EnvCell::EnvCell(Agent* agnt, unsigned int cx, unsigned int cy, EnvCellPayoffFunc fp, EnvCellCleanFunc fc)
    : x(cx)
    , y(cy)
    , m_agent(agnt)
{
    m_payoff_func.push_back(fp);
    m_clean_func.push_back(fc);
}

void EnvCell::addCellActivity(std::shared_ptr<Activity> aptr)
{
    /* The Activity must have this cell as an active cell: */
    double* t0s;
    double* t1s;
    int nts = aptr->getCellTimes(x, y, &t0s, &t1s);
    if(nts <= 0) {
        Log::err << "(" << x << "-" << y << ") Error adding activity " << *aptr << " in a cell, for \'"
            << aptr->getAgentId() << ":" << aptr->getId() << "\'.\n";
    } else {
        m_activities[aptr].t0s = new double[nts];
        m_activities[aptr].t1s = new double[nts];
        m_activities[aptr].nts = nts;
        for(int i = 0; i < nts; i++) {
            m_activities[aptr].t0s[i] = t0s[i];
            m_activities[aptr].t1s[i] = t1s[i];
        }
    }
}

bool EnvCell::removeCellActivity(std::shared_ptr<Activity> aptr)
{
    // Log::err << "Removing activity " << *aptr << " from cell " << *this << ".\n";
    auto it = m_activities.find(aptr);
    if(it != m_activities.end()) {
        if(it->second.nts > 0) {
            delete[] it->second.t0s;
            delete[] it->second.t1s;
        }
        m_activities.erase(it);
        return true;
    } else {
        /*  This activity might have been automatically cleaned from this cell with EnvCell::clean.
         **/
        // Log::warn << "The activity " << *aptr << " was no longer allocated in the cell " << *this << ".\n";
        return false;
    }
}

bool EnvCell::removeCellActivityById(std::string agent_id, unsigned int activity_id)
{
    for(auto it = m_activities.begin(); it != m_activities.end(); ) {
        if(it->first->getAgentId() == agent_id && it->first->getId() == (int)activity_id) {
            if(it->second.nts > 0) {
                delete[] it->second.t0s;
                delete[] it->second.t1s;
            }
            it = m_activities.erase(it);
            return true;
        } else {
            it++;
        }
    }
    return false;
}

bool EnvCell::updateCellActivity(std::shared_ptr<Activity> aptr)
{
    for(auto& a : m_activities) {
        if(a.first->getAgentId() == aptr->getAgentId() && a.first->getId() == aptr->getId()) {
            a.first->clone(aptr);
            return true;
        }
    }
    return false;
}


float EnvCell::computeCellPayoff(double* at0s, double* at1s, int nts)
{
    m_payoff.clear();
    for(int i = 0; i < nts; i++) {
        /*  Payoff function for one cell:
         *  Arg. #0:                   pair<double, double>  --> t0 & t1 of the potential new activity.
         *  Arg. #1: vector<vector<pair<double, double> > >  --> vector of t0 & t1 of the activities for this cell.
         *  Arg. #2:          vector<shared_ptr<Activity> >  --> Pointer to the activities (same index than arg2).
         **/
        auto arg1 = std::make_pair(at0s[i], at1s[i]);
        std::vector<std::vector<std::pair<double, double> > > arg2;
        std::vector<std::shared_ptr<Activity> > arg3;
        for(auto& ra : m_activities) {
            if(ra.first->isOwner(m_agent->getId()) && ra.first->getStartTime() > VirtualTime::now()) {
                /*  This activity is owned by the agent that is computing payoff and is in the future.
                 *  We will not consider it because we might be re-scheduling.
                 **/
                continue;
            }
            std::vector<std::pair<double, double> > vec_ts;
            for(int j = 0; j < ra.second.nts; j++) {
                vec_ts.push_back({ra.second.t0s[j], ra.second.t1s[j]});
            }
            arg2.push_back(vec_ts);
            arg3.push_back(ra.first);
        }
        /*  IMPORTANT NOTE:
         *  The following conditions are necessary:
         *  - Forwards revisit time payoff needs arg2/arg3 to be sorted with start time asc.
         *  - Backwards revisit time payoff needs arg2/arg3 to be sorted with end time asc.
         *  The conditions are always met, as long as EnvCellState objects are sorted. Given
         *  that these intervals are provided in an activity basis (i.e. several activities are
         *  not mixed, because we create independent vectors), then it is just a matter of
         *  ensuring that EnvCellState times are sorted.
         **/
        float po = 0.f;
        float uavg = 0.f;
        std::pair<float, float> po_uavg;
        for(unsigned int po_func_idx = 0; po_func_idx < m_payoff_func.size(); po_func_idx++) {
            po_uavg = m_payoff_func[po_func_idx](arg1, arg2, arg3);
            if(po_uavg.first > po) {
                po   = po_uavg.first;   /* Update payoff.       */
                uavg = po_uavg.second;  /* Update utility avg.  */
            }
        }
        m_payoff[at0s[i]].first  = po;      /* Set final value. */
        m_payoff[at0s[i]].second = uavg;    /* Set final value. */
    }
    return m_payoff.crbegin()->second.first;   /* Returns the "last" payoff. */
}

void EnvCell::getPayoff(double t, float& payoff, float& utility) const
{
    double t_diff;
    bool started = false;
    for(auto& po : m_payoff) {
        if(!started) {
            t_diff = std::abs(po.first - t);
            payoff  = po.second.first;
            utility = po.second.second;
            started = true;
        } else {
            if(std::abs(po.first - t) < t_diff) {
                t_diff = std::abs(po.first - t);
                payoff  = po.second.first;
                utility = po.second.second;
            }
        }
    }
    if(!started) {
        Log::warn << "Cell " << *this << " does not have payoffs to retrieve.\n";
        payoff  = -1.f;
        utility = -1.f;
    }
}

bool EnvCell::findActivity(std::shared_ptr<Activity> act) const
{
    return m_activities.find(act) != m_activities.end();
}

std::shared_ptr<Activity> EnvCell::getActivity(std::string agent_id, int activity_id) const
{
    for(auto& a : m_activities) {
        if(a.first->getAgentId() == agent_id && a.first->getId() == activity_id) {
            return a.first;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Activity> > EnvCell::getAllActivities(void) const
{
    std::vector<std::shared_ptr<Activity> > retval;
    for(auto& ac : m_activities) {
        retval.push_back(ac.first);
    }
    return retval;
}

void EnvCell::clean(double t)
{
    for(unsigned int fidx = 0; fidx < m_clean_func.size(); fidx++) {
        auto activities = m_clean_func[fidx](t, getAllActivities());
        for(auto& ac : activities) {
            removeCellActivity(ac);
        }
    }
}

std::set<std::pair<std::string, unsigned int> > EnvCell::getCellCrosscheckList(void) const
{
    std::set<std::pair<std::string, unsigned int> > retset;
    auto all_activities = getAllActivities();
    for(auto& ac : all_activities) {
        retset.insert(std::make_pair(ac->getAgentId(), ac->getId()));
    }
    return retset;
}

std::size_t EnvCell::pushPayoffFunc(const EnvCellPayoffFunc fp, const EnvCellCleanFunc fc)
{
    m_payoff_func.push_back(fp);
    m_clean_func.push_back(fc);
    return m_payoff_func.size() - 1;
}

std::size_t EnvCell::pushPayoffFunc(const std::pair<EnvCellPayoffFunc, EnvCellCleanFunc> f)
{
    return pushPayoffFunc(f.first, f.second);
}


std::ostream& operator<<(std::ostream& os, const EnvCell& ec)
{
    os << "(" << ec.x << "," << ec.y << ")[" << ec.m_payoff.size() << " PO";
    for(auto po : ec.m_payoff) {
        os << ":(" << po.first << "|" << po.second.first << "|" << po.second.second << ")";
    }
    os << "]";
    return os;
}
