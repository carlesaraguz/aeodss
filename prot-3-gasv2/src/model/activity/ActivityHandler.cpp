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

void ActivityHandler::discard(std::shared_ptr<Activity> pa)
{
    auto it = std::find(m_activities_own.begin(), m_activities_own.end(), pa);
    if(it != m_activities_own.end()) {
        pa->setDiscarded(true);
        buildActivityLUT();
    }
}


void ActivityHandler::purge(bool remove_unsent)
{
    bool bflag = false, report_flag = false;
    int count = 0;
    double tv_now = VirtualTime::now();
    double t_horizon = tv_now - Config::goal_target;
    for(auto it = m_activities_own.begin(); it != m_activities_own.end(); ) {
        auto act = *it;
        if((act->getEndTime() < t_horizon) || (remove_unsent && !act->isSent() && act->getStartTime() > tv_now)) {
            /* We shall remove it: */
            if(!act->isDiscarded()) {
                bflag = true;
            }
            report_flag = true;
            it = m_activities_own.erase(it);
            if(m_env_model_ptr != nullptr) {
                m_env_model_ptr->removeActivity(act);
            }
            count++;
        } else {
            it++;
        }
    }
    if(bflag) {
        buildActivityLUT();
    }
    Log::dbg << "Agent " << m_agent_id << " has purged " << count << " old activities (owned).\n";
    count = 0;
    for(auto& act_others : m_activities_others) {
        for(auto act_it = act_others.second.begin(); act_it != act_others.second.end(); ) {
            if(act_it->second->getEndTime() < t_horizon) {
                /* We shall remove it: */
                act_it = act_others.second.erase(act_it);
                bflag = true;
                report_flag = true;
                count++;
            } else {
                act_it++;
            }
        }
    }
    Log::dbg << "Agent " << m_agent_id << " has purged " << count << " old activities (from other agents).\n";
    if(report_flag) {
        report();
        bflag = false;
    }
}

void ActivityHandler::buildActivityLUT(void)
{
    m_act_own_lut.clear();
    for(unsigned int i = 0; i < m_activities_own.size(); i++) {
        if(!m_activities_own[i]->isDiscarded()) {
            m_act_own_lut[m_activities_own[i]->getStartTime()] = i;
        }
    }
}

void ActivityHandler::update(void)
{
    double t = VirtualTime::now();
    for(auto ait = m_activities_own.begin(); ait != m_activities_own.end(); ait++) {
        auto aptr = *ait;
        if(!aptr->isFact()) {
            /* Is neither discarded nor confirmed: */
            if(aptr->getStartTime() - t <= Config::activity_confirm_window) {
                /* Can be confirmed now: */
                aptr->setConfirmed(true);
            }
        }
    }
    buildActivityLUT();     /* REMOVE this after DEBUG */
    /* Checking overlaps: */
    if(m_act_own_lut.size() >= 2) {
        unsigned int j = m_act_own_lut.begin()->second;
        for(auto it = std::next(m_act_own_lut.begin()); it != m_act_own_lut.end(); it++) {
            if(!m_activities_own[it->second]->isDiscarded()) {
                if(isOverlapping(m_activities_own[it->second], m_activities_own[j])) {
                    std::string ida = std::to_string(m_activities_own[it->second]->getId());
                    std::string idb = std::to_string(m_activities_own[j]->getId());
                    Log::err << "Two non-discarded activities overlap in [" << m_agent_id << "]: " << ida << " and " << idb << "\n";
                    Log::err << "Will discard the older and continue, but this is unexpected\n";
                    if(m_activities_own[it->second]->getLastUpdateTime() >= m_activities_own[j]->getLastUpdateTime()) {
                        m_activities_own[j]->setDiscarded(true);
                    } else {
                        m_activities_own[it->second]->setDiscarded(true);
                    }
                }
                j = it->second;
            }
        }
    }
}

bool ActivityHandler::isOverlapping(std::shared_ptr<Activity> a, std::shared_ptr<Activity> b) const
{
    double tsa = a->getStartTime();
    double tea = a->getEndTime();
    double tsb = b->getStartTime();
    double teb = b->getEndTime();
    if((tsa <= tsb && tsb < tea) || (tsb <= tsa && tsa < teb)) {
        return true;
    } else if((tsb >= tsa && tsb < tea && teb <= tea && teb > tsa) || (tsa >= tsb && tsa < teb && tea <= teb && tea > tsb)) {
        return true;
    } else {
        return false;
    }
}

std::vector<std::shared_ptr<Activity> > ActivityHandler::checkOverlaps(std::shared_ptr<Activity> a, std::vector<std::shared_ptr<Activity> >& beta)
{
    std::vector<std::shared_ptr<Activity> > retvec;
    if(a->isDiscarded()) {
        return retvec;
    }
    for(auto& bi : beta) {
        if(!bi->isDiscarded()) {
            if(isOverlapping(a, bi)) {
                Log::err << "Activity [" << a->getAgentId() << ":" << a->getId() << "] overlaps with [" << bi->getAgentId() << ":" << bi->getId() << "]\n";
                retvec.push_back(bi);
            }
        }
    }
    return retvec;
}

void ActivityHandler::markAsSent(int aid)
{
    bool found = false;
    if(aid >= 0) {
        for(unsigned int idx = 0; idx < m_activities_own.size(); idx++) {
            if(m_activities_own[idx]->getId() == aid) {
                m_activities_own[idx]->markAsSent();
                found = true;
            }
        }
    }
    if(!found) {
        Log::err << "Trying to mark an activity as sent, but is no longer in the knowledge base: [" << m_agent_id << ":" << aid << "]\n";
    }
}

bool ActivityHandler::isCapturing(void)
{
    double t = VirtualTime::now();
    for(unsigned int idx = 0; idx < m_activities_own.size(); idx++) {
        if( m_activities_own[idx]->getStartTime() <= t  &&
            m_activities_own[idx]->getEndTime() > t     &&
            !m_activities_own[idx]->isDiscarded()
        ) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<Activity> ActivityHandler::getNextActivity(double t) // const
{
    if(t <= -1.0) {
        t = VirtualTime::now();
    }

    std::shared_ptr<Activity> retval(nullptr);
    /*  NB: This function assumes that:
     *  - Activities are sorted by start time.
     *  - Start and end times of activities for an agent do not overlap.
     **/
    buildActivityLUT();
    for(auto& idx : m_act_own_lut) {
        auto ac = m_activities_own[idx.second];
        if(ac->getStartTime() > t && !ac->isDiscarded()) {
            retval = m_activities_own[idx.second];
            break;
        }
    }
    return retval;
}

std::shared_ptr<Activity> ActivityHandler::getCurrentActivity(void) // const
{
    std::shared_ptr<Activity> retval(nullptr);
    double t = VirtualTime::now();
    /*  NB: This function assumes that:
     *  - Activities are sorted by start time.
     *  - Start and end times of activities for an agent do not overlap.
     **/
    buildActivityLUT();
    for(auto& idx : m_act_own_lut) {
        auto ac = m_activities_own[idx.second];
        if(ac->getStartTime() <= t && ac->getEndTime() >= t && !ac->isDiscarded()) {
            retval = ac;
            break;
        }
    }
    return retval;
}

std::vector<std::shared_ptr<Activity> > ActivityHandler::getPending(void) // const
{
    std::vector<std::shared_ptr<Activity> > retvec;
    double t = VirtualTime::now();
    /*  NB: This function assumes that:
     *  - Activities are sorted by start time (iterating the LUT/std::map guarantees this).
     *  - The LUT only has confirmed and undecided activities (i.e. not discarded).
     **/
    buildActivityLUT();
    for(auto& idx : m_act_own_lut) {
        auto ac = m_activities_own[idx.second];
        if(ac->getStartTime() >= t) {
            retvec.push_back(ac);
        } else {
        }
    }
    return retvec;
}

std::shared_ptr<Activity> ActivityHandler::getLastActivity(void) // const
{
    buildActivityLUT();
    if(m_activities_own.size() > 0) {
        return m_activities_own[m_act_own_lut.rbegin()->second];
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
        if(a->getEndTime() > VirtualTime::now() && !a->isDiscarded()) {   /* Ends in the future. */
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

// void ActivityHandler::add(std::shared_ptr<Activity> a, std::vector<std::shared_ptr<Activity> >& beta)

void ActivityHandler::add(std::shared_ptr<Activity> pa)
{
    if(pa->getActiveCells().size() == 0) {
        Log::err << "[" << m_agent_id << "] Trying to add an activity that had 0 active cells: " << pa << ". Ignoring.\n";
        return;
    }
    if(pa->isOwner(m_agent_id)) {
        /* It's owned: */
        pa->setId(m_activity_count++);
        auto overlap_vec = checkOverlaps(pa, m_activities_own);
        if(m_act_own_lut.find(pa->getStartTime()) == m_act_own_lut.end() && overlap_vec.size() == 0) {
            m_activities_own.push_back(pa);
            if(m_env_model_ptr != nullptr) {
                m_env_model_ptr->addActivity(pa);
            }
            m_act_own_lut[pa->getStartTime()] = m_activities_own.size() - 1;
            Log::dbg << "Agent " << m_agent_id << " added a new activity: " << *pa << "\n";
        } else {
            Log::err << "Agent " << m_agent_id << " was trying to add an overlapping activity: " << *pa << "\n";
        }
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
                aptr->setConfidence();  /* Updates the confidence that will be reported if undecided. */
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
