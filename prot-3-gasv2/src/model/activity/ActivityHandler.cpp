/***********************************************************************************************//**
 *  Container and manager of activities known or generated by a single Agent.
 *  @class      ActivityHandler
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-26
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "ActivityHandler.hpp"
#include "Agent.hpp"

CREATE_LOGGER(ActivityHandler)

ActivityHandler::ActivityHandler(Agent* aptr)
    : ReportGenerator(false)
    , m_agent(aptr)
    , m_activity_count(0)
    , m_update_view(false)
    , m_current_activity_idx(-1)
    , m_aperture(0.f)
{
    m_self_view.setOwnActivityList(&m_activities_own);
    m_self_view.setOthersActivityList(&m_activities_others);
    m_self_view.display(ActivityDisplayType::NONE);
}

void ActivityHandler::setAgentId(std::string aid)
{
    m_agent_id = aid;
    m_self_view.setAgentId(aid);
    initReport("agents/" + aid + "/", "knowledgebase.csv");
    addReportColumn("known_facts_own");     /* 0 */
    addReportColumn("known_facts_others");  /* 1 */
    addReportColumn("confirmed_own");       /* 2 */
    addReportColumn("confirmed_others");    /* 3 */
    addReportColumn("undecided_own");       /* 4 */
    addReportColumn("undecided_others");    /* 5 */
}

void ActivityHandler::report(void)
{
    if(!isReportEnabled()) {
        return;
    }
    int count_confirmed_own = 0;
    int count_facts_own = 0;
    int count_undecided_own = 0;
    for(auto& ac : m_activities_own) {
        if(ac->isFact()) {
            count_facts_own++;
            if(ac->isConfimed()) {
                count_confirmed_own++;
            }
        } else {
            count_undecided_own++;
        }
    }
    int count_confirmed_others = 0;
    int count_facts_others = 0;
    int count_undecided_others = 0;
    for(auto& acmap : m_activities_others) {
        for(auto& ac : acmap.second) {
            if(ac.second->isFact()) {
                count_facts_others++;
                if(ac.second->isConfimed()) {
                    count_confirmed_others++;
                }
            } else {
                count_undecided_others++;
            }
        }
    }
    setReportColumnValue(0, count_facts_own);
    setReportColumnValue(1, count_facts_others);
    setReportColumnValue(2, count_confirmed_own);
    setReportColumnValue(3, count_confirmed_others);
    setReportColumnValue(4, count_undecided_own);
    setReportColumnValue(5, count_undecided_others);
}

void ActivityHandler::purge(void)
{
    bool bflag = false;
    double t_horizon = VirtualTime::now() - Config::goal_target;
    for(auto it = m_activities_own.begin(); it != m_activities_own.end(); ) {
        if((*it)->getEndTime() < t_horizon) {
            /* We shall remove it: */
            it = m_activities_own.erase(it);
            m_current_activity_idx -= 1;
            bflag = true;
        } else {
            /* We're done, provided that activities are properly sorted. */
            break;
        }
    }
    for(auto& act_others : m_activities_others) {
        for(auto act_it = act_others.second.begin(); act_it != act_others.second.end(); ) {
            if(act_it->second->getEndTime() < t_horizon) {
                /* We shall remove it: */
                act_it = act_others.second.erase(act_it);
                bflag = true;
            } else {
                act_it++;
            }
        }
    }
    if(bflag) {
        report();
        bflag = false;
    }
}

void ActivityHandler::update(void)
{
    double t = VirtualTime::now();
    if(m_current_activity_idx < 0) {
        /* We have no clue about the current activity: */
        for(unsigned int idx = 0; idx < m_activities_own.size(); idx++) {
            if(m_activities_own[idx]->getStartTime() <= t && m_activities_own[idx]->getEndTime() < t) {
                /* This is the current activity: */
                m_current_activity_idx = idx;
                break;
            }
        }
    } else {
        /* There was an index set before: */
        int previdx = m_current_activity_idx;
        m_current_activity_idx = -1;
        if(previdx < (int)m_activities_own.size()) {
            auto aptr = m_activities_own[previdx];
            if(aptr->getEndTime() <= t) {
                for(int idx = previdx; idx < (int)m_activities_own.size(); idx++) {
                    if(m_activities_own[idx]->getStartTime() <= t && m_activities_own[idx]->getEndTime() < t) {
                        m_current_activity_idx = idx;
                        break;
                    }
                }
            } else if(aptr->getStartTime() > t) {
                for(int idx = previdx; idx >= 0; idx--) {
                    if(m_activities_own[idx]->getStartTime() <= t && m_activities_own[idx]->getEndTime() < t) {
                        m_current_activity_idx = idx;
                        break;
                    }
                }
            } else {
                /* The index is still valid. */
                m_current_activity_idx = previdx;
            }
        }
    }
    if(m_current_activity_idx >= 0) {
        m_activities_own[m_current_activity_idx]->setConfirmed();
        if(m_current_activity_idx + 1 < (int)m_activities_own.size()) {
            /* Also confirm the next one: */
            m_activities_own[m_current_activity_idx + 1]->setConfirmed();
        }
    }
}

bool ActivityHandler::isCapturing(void)
{
    double t = VirtualTime::now();
    if(m_current_activity_idx >= 0 && m_current_activity_idx < (int)m_activities_own.size()) {
        auto aptr = m_activities_own.at(m_current_activity_idx);
        if(aptr->getStartTime() <= t && aptr->getEndTime() > t) {
            return true;
        } else if(aptr->getEndTime() <= t) {
            /* The current activity has ended, check for next: */
            if(m_current_activity_idx + 1 < (int)m_activities_own.size()) {
                aptr = m_activities_own.at(m_current_activity_idx + 1);
                if(aptr->getStartTime() <= t && aptr->getEndTime() > t) {
                    /* This is the new current activity: */
                    m_current_activity_idx++;
                    return true;
                } else if(aptr->getStartTime() > t) {
                    /* The index is still valid, but next activity has not started: */
                    return false;
                }
            }
        } /* ...else the index is no longer valid for some reason. */
        m_current_activity_idx = -1;
    }
    for(unsigned int idx = 0; idx < m_activities_own.size(); idx++) {
        if(m_activities_own[idx]->getStartTime() <= t && m_activities_own[idx]->getEndTime() > t) {
            m_current_activity_idx = idx;
            return true;
        }
    }
    return false;
}

std::shared_ptr<Activity> ActivityHandler::getNextActivity(double t) const
{
    if(t <= -1.0) {
        t = VirtualTime::now();
    }

    std::shared_ptr<Activity> retval(nullptr);
    /*  NB: This function assumes that:
     *  - Activities are sorted by start time.
     *  - Start and end times of activities for an agent do not overlap.
     **/
    for(auto& ac : m_activities_own) {
        if(ac->getStartTime() > t) {
            retval = ac;
            break;
        }
    }
    return retval;
}

std::shared_ptr<Activity> ActivityHandler::getCurrentActivity(void) const
{
    std::shared_ptr<Activity> retval(nullptr);
    double t = VirtualTime::now();
    /*  NB: This function assumes that:
     *  - Activities are sorted by start time.
     *  - Start and end times of activities for an agent do not overlap.
     **/
    for(auto& ac : m_activities_own) {
        if(ac->getStartTime() <= t && ac->getEndTime() >= t) {
            retval = ac;
            break;
        }
    }
    return retval;
}

std::shared_ptr<Activity> ActivityHandler::getLastActivity(void) const
{
    if(m_activities_own.size() > 0) {
        return m_activities_own.back();
    } else {
        return nullptr;
    }
}

unsigned int ActivityHandler::count(std::string aid) const
{
    if(aid == m_agent_id) {
        return m_activities_own.size();
    } else {
        auto it = m_activities_others.find(aid);
        if(it != m_activities_others.end()) {
            return m_activities_others.at(aid).size();
        } else {
            return 0;
        }
    }
}

unsigned int ActivityHandler::pending(void) const
{
    int count = 0;
    for(auto& a : m_activities_own) {
        if(a->getEndTime() > VirtualTime::now()) {       /* Ends in the future. */
            count++;
        }
    }
    return count;
}


std::shared_ptr<Activity> ActivityHandler::createOwnedActivity(
    const std::map<double, sf::Vector3f>& a_pos,
    const std::vector<ActivityCell>& a_cells)
{
    auto a = std::make_shared<Activity>(m_agent_id);
    a->setAperture(m_aperture);
    a->setTrajectory(a_pos, a_cells);
    return a;
}

void ActivityHandler::add(std::shared_ptr<Activity> pa)
{
    if(pa->getActiveCells().size() == 0) {
        Log::err << "[" << m_agent_id << "] Trying to add an activity that had 0 active cells: " << pa << ". Ignoring.\n";
        return;
    }
    if(pa->isOwner(m_agent_id)) {
        /* It's owned: */
        pa->setId(m_activity_count++);
        m_activities_own.push_back(pa);
        if(m_env_model_ptr != nullptr) {
            m_env_model_ptr->addActivity(pa);
        }
        if(m_activities_own.size() == 1) {
            m_current_activity_idx = 0;
        } else if(m_current_activity_idx == (int)m_activities_own.size() - 2) {
            m_current_activity_idx = m_activities_own.size() - 1;
        }
        std::sort(m_activities_own.begin(), m_activities_own.end());
        Log::dbg << "Agent " << m_agent_id << " added a new activity: " << *pa << "\n";
    } else {
        /* It has been received: */
        if(m_activities_others[pa->getAgentId()][pa->getId()] != nullptr) {
            if(m_activities_others[pa->getAgentId()][pa->getId()]->getLastUpdateTime() < pa->getLastUpdateTime()) {
                m_activities_others[pa->getAgentId()][pa->getId()] = pa;
                if(m_env_model_ptr != nullptr) {
                    m_env_model_ptr->updateActivity(pa);
                }
                Log::dbg << "Agent " << m_agent_id << " updated an activity from " << pa->getAgentId() << ": " << *pa << "\n";
            }
        } else {
            m_activities_others[pa->getAgentId()][pa->getId()] = pa;
            if(m_env_model_ptr != nullptr) {
                m_env_model_ptr->addActivity(pa);
            }
            Log::dbg << "Agent " << m_agent_id << " added an new activity from " << pa->getAgentId() << ": " << *pa << "\n";
        }
    }
    if(m_update_view) {
        m_self_view.update();
    }
}

const sf::Drawable& ActivityHandler::getView(void) const
{
    return m_self_view;
}

void ActivityHandler::autoUpdateView(bool auto_update)
{
    m_update_view = auto_update;
}

void ActivityHandler::displayInView(ActivityDisplayType adt, std::vector<std::pair<std::string, unsigned int> > filter)
{
    m_self_view.display(adt, filter);
}

std::vector<std::shared_ptr<Activity> > ActivityHandler::getActivitiesToExchange(std::string aid)
{
    std::vector<std::shared_ptr<Activity> > retvec;
    double time_th = VirtualTime::now() - Config::goal_target;     /* Time threshold. */

    /* Start by including all owned activities that are relevant: */
    if(aid != m_agent_id) {
        for(auto aptr : m_activities_own) {
            if(aptr->getEndTime() >= time_th) {
                retvec.push_back(aptr);
            }
        }
    }
    /* Add all activities from others (except those from aid itself): */
    for(auto& as_others : m_activities_others) {
        if(as_others.first != aid) {
            for(auto& apair : as_others.second) {
                if(apair.second->getEndTime() >= time_th) {
                    retvec.push_back(apair.second);
                }
            }
        }
    }
    /* Sort by priority: */
    std::sort(retvec.begin(), retvec.end(), [](std::shared_ptr<Activity> a, std::shared_ptr<Activity> b) {
        return a->getPriority(ActivityPriorityModel::BASIC) > b->getPriority(ActivityPriorityModel::BASIC);
    });

    /* Debug:
    Log::warn << "==== Agent " << m_agent_id << " is going to transfer the following activities to " << aid << ": =====================\n";
    int count = 0;
    for(auto& a : retvec) {
        if(count++ <= 20) {
            Log::warn << std::setw(2) << count++ << ": " << std::fixed << std::setprecision(6) << std::setw(8)
            << a->getPriority(ActivityPriorityModel::BASIC) << " === " << *a << ".\n";
        } else {
            break;
        }
    }
    Log::warn << std::defaultfloat; */

    /* Limit the number of activities to exchange: */
    if(retvec.size() > 20) {
        return std::vector<std::shared_ptr<Activity> >(retvec.begin(), retvec.begin() + 20);
    } else {
        return retvec;
    }
}
