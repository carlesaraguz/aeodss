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

void EnvCell::addActivity(std::shared_ptr<Activity> aptr)
{
    /* The Activity must have this cell as an active cell: */
    float t0, t1;
    aptr->getCellTimes(x, y, t0, t1);
    if(t0 == -1.f || t1 == -1.f) {
        Log::err << "(" << x << "-" << y << ") Error adding activity \'"
            << aptr->getAgentId() << ":" << aptr->getId() << "\'.\n";
    } else {
        m_activities[aptr] = {t0, t1};
    }
}

void EnvCell::removeActivity(std::shared_ptr<Activity> aptr)
{
    auto it = m_activities.find(aptr);
    if(it != m_activities.end()) {
        m_activities.erase(it);
    } else {
        Log::err << "(" << x << "-" << y << ") Error removing activity \'"
            << aptr->getAgentId() << ":" << aptr->getId() << "\'.\n";
    }
}

float EnvCell::computeCellPayoff(unsigned int fidx, std::shared_ptr<Activity> aptr) const
{
    if(fidx < m_payoff_func.size()) {
        /*  Payoff function for one cell:
         *  Arg. #0:          shared_ptr<Activity>  --> The potential new activity.
         *  Arg. #1:            pair<float, float>  --> t0 & t1 of the potential new activity.
         *  Arg. #2:   vector<pair<float, float> >  --> t0 & t1 of the activities for this cell.
         *  Arg. #3: vector<shared_ptr<Activity> >  --> Pointer to the activities (same index than arg2).
         **/
        float at0, at1;
        aptr->getCellTimes(x, y, at0, at1);
        auto arg1 = std::make_pair(at0, at1);
        std::vector<std::pair<float, float> > arg2;
        std::vector<std::shared_ptr<Activity> > arg3;
        for(auto& ra : m_activities) {
            arg2.push_back({ra.second.t0, ra.second.t1});
            arg3.push_back(ra.first);
        }
        return m_payoff_func[fidx](aptr, arg1, arg2, arg3);
    }
    Log::err << "(" << x << "-" << y << ") Error calculating payoff, wrong function index.\n";
    return 0.f;
}

std::vector<std::shared_ptr<Activity> > EnvCell::getAllActivities(void) const
{
    std::vector<std::shared_ptr<Activity> > retval;
    for(auto& ac : m_activities) {
        retval.push_back(ac.first);
    }
    return retval;
}

void EnvCell::clean(unsigned int fidx, float t)
{
    auto activities = m_clean_func[fidx](t, getAllActivities());
    for(auto& ac : activities) {
        m_activities.erase(m_activities.find(ac));
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
