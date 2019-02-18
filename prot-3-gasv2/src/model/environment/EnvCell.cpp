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

CREATE_LOGGER(EnvCell)

EnvCell::EnvCell(unsigned int cx, unsigned int cy)
    : x(cx)
    , y(cy)
{ }

EnvCell::EnvCell(unsigned int cx, unsigned int cy, EnvCellPayoffFunc fp, EnvCellCleanFunc fc)
    : x(cx)
    , y(cy)
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
        return false;
    }
}

bool EnvCell::removeCellActivityById(std::string agent_id, unsigned int activity_id)
{
    for(auto it = m_activities.begin(); it != m_activities.end(); ) {
        if(it->first->getAgentId() == agent_id && it->first->getId() == activity_id) {
            if(it->second.nts > 0) {
                delete[] it->second.t0s;
                delete[] it->second.t1s;
            }
            it = m_activities.erase(it);
            break;
        } else {
            it++;
        }
    }
}

bool EnvCell::updateCellActivity(std::shared_ptr<Activity> aptr)
{
    for(auto& a : m_activities) {
        if(a.first->getAgentId() == aptr->getAgentId() && a.first->getId() == aptr->getId()) {
            a.first->clone(aptr);
            break;
        }
    }
}


float EnvCell::computeCellPayoff(unsigned int fidx, double* at0s, double* at1s, int nts)
{
    if(fidx < m_payoff_func.size() && nts > 0) {
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
                std::vector<std::pair<double, double> > vec_ts;
                for(int j = 0; j < ra.second.nts; j++) {
                    vec_ts.push_back({ra.second.t0s[j], ra.second.t1s[j]});
                }
                arg2.push_back(vec_ts);
                arg3.push_back(ra.first);
            }
            m_payoff[at0s[i]] = m_payoff_func[fidx](arg1, arg2, arg3);
        }
        return m_payoff.crbegin()->second;   /* Returns the "last" payoff. */
    } if(fidx >= m_payoff_func.size()) {
        Log::err << *this << " Error calculating payoff, wrong function index.\n";
    }
    return -1.f;
}

float EnvCell::getPayoff(double t) const
{
    double t_diff, retval;
    bool started = false;
    for(auto& po : m_payoff) {
        if(!started) {
            t_diff = std::abs(po.first - t);
            retval = po.second;
            started = true;
        } else {
            if(std::abs(po.first - t) < t_diff) {
                t_diff = std::abs(po.first - t);
                retval = po.second;
            }
        }
    }
    if(!started) {
        Log::warn << "Cell " << *this << " does not have payoffs to retrieve.\n";
        return -1.0;
    } else {
        return retval;
    }
}

std::vector<std::shared_ptr<Activity> > EnvCell::getAllActivities(void) const
{
    std::vector<std::shared_ptr<Activity> > retval;
    for(auto& ac : m_activities) {
        retval.push_back(ac.first);
    }
    return retval;
}

void EnvCell::clean(unsigned int fidx, double t)
{
    auto activities = m_clean_func[fidx](t, getAllActivities());
    for(auto& ac : activities) {
        removeCellActivity(ac);
    }
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
        os << ":(" << po.first << "|" << po.second << ")";
    }
    os << "]";
    return os;
}
