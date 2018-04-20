/***********************************************************************************************//**
 *  Handler and store of intents.
 *  @class      IntentHandler
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-06
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "IntentHandler.hpp"

const IntentHandler::Opts IntentHandler::select_all = {
    IntentSelection::ALL,   /* .filter      */
    -1,                     /* .n_intents   */
    0.f,                    /* .time        */
    ""                      /* .aid         */
};

IntentHandler::IntentHandler(std::string aid)
    : m_agent_id(aid)
    , m_time(0.f)
{ }

IntentHandler::IntentTable IntentHandler::getIntents(void) const
{
    Opts o;
    o.filter = IntentSelection::ALL;
    o.n_intents = -1;
    o.time = 0.f;
    o.aid = "";
    return getIntents(o);
}

IntentHandler::IntentTable IntentHandler::getIntents(Opts opt, const IntentTable* exclude_list) const
{
    bool single_agent = false;
    bool only_owned   = false;
    bool only_future  = false;
    bool has_agent_id = false;
    bool do_exclude   = (exclude_list != nullptr);

    if(opt.filter != IntentSelection::ALL) {
        only_owned = !((opt.filter & IntentSelection::OWN) == IntentSelection::OWN);
        if(only_owned) {
            opt.aid = m_agent_id;
        }
        only_future  = ((opt.filter & IntentSelection::FUTURE) == IntentSelection::FUTURE);
        single_agent = (opt.aid.length() != 0);
        has_agent_id = (m_intents.find(opt.aid) != m_intents.end());
    }

    int count = 0;
    IntentTable retval;
    if(single_agent && has_agent_id) {
        for(auto& is : m_intents.at(opt.aid)) {
            if(only_future && opt.time >= is.second.tstart) {
                continue;
            } else if(do_exclude) {
                if(exclude_list->find(opt.aid) != exclude_list->end()) {
                    if(exclude_list->at(opt.aid).find(is.second.id) != exclude_list->at(opt.aid).end()) {
                        /* This intent is in the exclude list. */
                        continue;
                    }
                }
            }
            if(((opt.n_intents > 0) && (count++ < opt.n_intents)) || (opt.n_intents <= 0)) {
                retval[opt.aid][is.second.id] = is.second;
            } else {
                break;
            }
        }
    } else if(single_agent && !has_agent_id) {
        // std::cerr << "IntentHandler error: intents from agent \'" << opt.aid << "\' are not available.\n";
    } else {
        for(auto& ina : m_intents) {
            for(auto& is : ina.second) {
                if(only_future && opt.time >= is.second.tstart) {
                    continue;
                } else if(do_exclude) {
                    if(exclude_list->find(ina.first) != exclude_list->end()) {
                        if(exclude_list->at(ina.first).find(is.second.id) != exclude_list->at(ina.first).end()) {
                            /* This intent is in the exclude list. */
                            continue;
                        }
                    }
                }
                if(((opt.n_intents > 0) && (count++ < opt.n_intents)) || (opt.n_intents <= 0)) {
                    retval[ina.first][is.second.id] = is.second;
                } else {
                    break;
                }
            }
        }
    }
    return retval;
}

bool IntentHandler::processRcvIntents(const IntentTable& isas)
{
    bool retval = false;
    for(auto& is : isas) {
        if(m_intents.find(is.first) == m_intents.end()) {
            /* New agent: no intents from this Agent were registered. Insert all of them at once. */
            retval = true;
            addNewIntent(is.second.begin(), is.second.end(), is.first);
        } else {
            /* Known agent: insert non-available intents. */
            for(auto j = is.second.begin(); j != is.second.end(); j++) {
                if(m_intents[is.first].find(j->first) == m_intents[is.first].end()) {
                    addNewIntent(j->second, is.first);
                    retval = true;
                } else {
                    retval |= updateIntent(j->second, is.first);
                }
            }
        }
    }
    return retval;
}

void IntentHandler::createIntent(Intent i)
{
    m_intents[m_agent_id].emplace(i.id, i);
    SegmentView segment(i);
    segment.setOwnership(true);
    m_segments[m_agent_id].emplace(i.id, segment);
}

void IntentHandler::addNewIntent(IntentTableElemIt ib, IntentTableElemIt ie, std::string aid)
{
    for(auto j = ib; j != ie; j++) {
        m_intents[aid].emplace(*j);
        SegmentView segment(j->second);
        segment.setOwnership(false);
        m_segments[aid].emplace(std::make_pair(j->second.id, segment));
    }
}

void IntentHandler::addNewIntent(Intent i, std::string aid)
{
    m_intents[aid][i.id] = i;
    SegmentView segment(i);
    segment.setOwnership(false);
    m_segments[aid].emplace(std::make_pair(i.id, segment));
}

bool IntentHandler::updateIntent(Intent /* i */, std::string /* aid */)
{
    /* Do nothing. */
    return false;
}

int IntentHandler::getIntentCount(std::string aid, float now) const
{
    if(m_intents.find(aid) != m_intents.end()) {
        int count = 0;
        for(auto& j : m_intents.at(aid)) {
            if(now < j.second.tend) {
                count++;
            }
        }
        return count;
    } else {
        return 0;
    }
}


int IntentHandler::getTotalIntentCount(void) const
{
    using TableItem = std::pair<std::string, std::map<unsigned int, Intent> >;
    return std::accumulate(m_intents.cbegin(), m_intents.cend(), 0, [](int acc, TableItem el) {
        return acc + el.second.size();
    });
}

int IntentHandler::getActiveIntentsAt(float t, std::string aid) const
{
    if(m_intents.find(aid) != m_intents.end()) {
        int acc = 0;
        for(auto& j : m_intents.at(aid)) {
            if(t >= j.second.tstart && t < j.second.tend) {
                acc++;
            }
        }
        return acc;
    } else {
        return 0;
    }
}

std::vector<std::tuple<sf::Vector2f, float> > IntentHandler::getActivePositions(float t) const
{
    if(t <= -1.f) {
        t = m_time;
    }
    std::vector<std::tuple<sf::Vector2f, float> > retvec;
    for(auto& agis : m_intents) {
        for(auto& i : agis.second) {
            if(i.second.tstart <= t && i.second.tend > t) {
                /* This intent is the active one for this agent: */
                retvec.push_back(std::make_tuple(
                    i.second.getPositionAt(t),
                    i.second.getAgentSwath()
                ));
                break;
            }
        }
    }
    return retvec;
}

void IntentHandler::disposeIntents(float t)
{
    std::vector<unsigned int> intent_ids;
    for(auto& agis : m_intents) {
        for(auto& i : agis.second) {
            if(i.second.tend <= t) {
                intent_ids.push_back(i.first);
            }
        }
        for(unsigned int j = 0; j < intent_ids.size(); j++) {
            /* Dispose the old intent: */
            agis.second.erase(agis.second.find(intent_ids[j]));
            m_segments[agis.first].erase(m_segments[agis.first].find(intent_ids[j]));
        }
        intent_ids.clear();
    }
}


bool IntentHandler::isActiveAt(float t)
{
    if(m_intents.find(m_agent_id) == m_intents.end()) {
        return false;
    }
    bool retval = false;
    for(auto& i : m_intents.at(m_agent_id)) {
        if(t >= i.second.tstart && t <= i.second.tend) {
            m_segments.at(m_agent_id).at(i.second.id).setActive(true);
            retval = true;
        } else if(t > i.second.tend) {
            m_segments.at(m_agent_id).at(i.second.id).setDone(true);
        }
    }
    return retval;
}

float IntentHandler::getLastIntentTime(std::string aid) const
{
    if(m_intents.find(aid) != m_intents.end()) {
        return m_intents.at(aid).rbegin()->second.tend;
    }
    return 0.f;
}
