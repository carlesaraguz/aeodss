/***********************************************************************************************//**
 *  The communications device of an agent.
 *  @class      AgentLink
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-24
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "AgentLink.hpp"

CREATE_LOGGER(AgentLink)

AgentLink::AgentLink(Agent* aptr)
    : AgentLink(aptr, Random::getUf(Config::agent_range_min, Config::agent_range_max))
{ }

AgentLink::AgentLink(Agent* aptr, float range)
    : AgentLink(aptr, range, Random::getUf(Config::agent_datarate_min, Config::agent_datarate_max))
{ }

AgentLink::AgentLink(Agent* aptr, float range, float datarate)
    : m_enabled(false)
    , m_range(range)
    , m_datarate(datarate)
    , m_energy_consumed(0.f)
    , m_agent(aptr)
    , m_tx_count(0)
    , m_encounter_callback([](std::string) -> bool { return true; })
    , m_connected_callback([](std::string) { })
{ }


void AgentLink::setAgents(std::vector<std::shared_ptr<Agent> > agents)
{
    for(auto& aptr : agents) {
        if(*aptr != *m_agent) {
            m_other_agents[aptr->getId()] = aptr;
        }
    }
}

void AgentLink::setPosition(sf::Vector2f p)
{
    m_position = sf::Vector3f(p.x, p.y, 0.f);
    m_self_view.setPosition(m_position);
}

void AgentLink::setPosition(sf::Vector3f p)
{
    m_position = p;
    m_self_view.setPosition(m_position);
}


bool AgentLink::tryConnect(std::shared_ptr<AgentLink> other)
{
    if(m_enabled) {
        std::string aid = other->getAgentId();
        if(m_connected[aid]) {
            /* Already connected to this agent. Do nothing. */
            Log::warn << "Agent " << aid << " is trying to connect to "
                << getAgentId() << " but they were already connected.\n";
            return true;
        } else {
            if(m_encounter_callback(aid)) {
                doConnect(other->getAgentId());
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

void AgentLink::notifyDisconnect(std::string aid_other)
{
    doDisconnect(aid_other);
}

void AgentLink::doConnect(std::string aid)
{
    /* Create an empty vector of Transfer's in TX and RX queues if it doesn't exist. */
    if(m_tx_queue.find(aid) == m_tx_queue.end()) {
        m_tx_queue[aid] = std::vector<Transfer>();
    }
    if(m_rx_queue.find(aid) == m_rx_queue.end()) {
        m_rx_queue[aid] = std::vector<Transfer>();
    }
    float r = m_other_agents[aid]->getLink()->getRange();
    m_link_ranges[aid] = std::min(r, m_range);
    m_connected[aid] = true;
    m_connected_callback(aid);
    m_self_view.setLink(aid, AgentLinkView::State::CONNECTED, m_other_agents[aid]->getMotion().getPosition());
}

void AgentLink::doDisconnect(std::string aid)
{
    if(!m_connected[aid]) {
        Log::warn << "Agent " << getAgentId() << ", trying to disconnect from " << aid
            << " but was already disconnected.\n";
        return;
    }

    /* Cancel scheduled or on-going TX transfers with this agent: */
    for(auto& tx : m_tx_queue.at(aid)) {
        if(!tx.finished && tx.started) {
            // Log::dbg << "Agent " << getAgentId() << " will cancel the on-going transfer " << tx.id << ".\n";
            m_other_agents[aid]->getLink()->cancelTransfer(getAgentId(), tx);  /* Cancel this transfer. */
        } else if(!tx.finished && !tx.started) {
            /* We don't have to do anything. */
            // Log::dbg << "Agent " << getAgentId() << " had a tranfer scheduled that did not start: " << tx.id << ".\n";
        } else {
            Log::err << "Agent " << getAgentId() << ", while disconnecting from " << aid
                << " found an error in the TX queue (1).\n";
        }
        auto txcb = m_callback_failure.find(tx.id);
        if(txcb != m_callback_failure.end()) {
            m_callback_failure[tx.id](tx.id);
            m_callback_failure.erase(txcb);
        } else {
            Log::err << "Agent " << getAgentId() << ", while disconnecting from " << aid
                << " found an error in the TX queue (2).\n";
        }
    }
    std::vector<Transfer> empty_tx_queue;
    m_tx_queue[aid].swap(empty_tx_queue);

    /* Remove information: */
    m_link_ranges.erase(m_link_ranges.find(aid));
    try {
        if(m_rx_queue.at(aid).size() > 0) {
            std::vector<Transfer> only_finished;
            for(auto& rxt : m_rx_queue.at(aid)) {
                if(rxt.finished) {
                    only_finished.push_back(rxt);
                }
            }
            m_rx_queue.at(aid).swap(only_finished);
        }
    } catch(std::out_of_range& e) {
        Log::err << "Agent " << getAgentId() << ", while disconnecting from " << aid
            << " found an error in the control structures.\n";
    }
    m_connected[aid] = false;
    m_self_view.setLink(aid, AgentLinkView::State::DISCONNECTED);

    if(Config::verbosity) {
        Log::dbg << "Agent " << getAgentId() << " has disconnected from " << aid << ".\n";
    }
}

void AgentLink::update(void)
{
    /* Disconnect from agents that are no longer in range. */
    std::vector<std::string> disconnect_list;
    for(auto& l : m_connected) {
        if(l.second) {
            if(!hasLineOfSight(m_other_agents[l.first]) || !isInRange(m_other_agents[l.first])) {
                disconnect_list.push_back(l.first);
            }
        }
    }
    for(auto& aid : disconnect_list) {
        if(Config::verbosity) {
            Log::dbg << "Agent " << getAgentId() << " is going to disconnect from " << aid << ".\n";
        }
        doDisconnect(aid);
        /* Notify of disconnection: */
        m_other_agents[aid]->getLink()->notifyDisconnect(getAgentId());
    }

    /*  Find agents that are now in range: */
    for(auto& a : m_other_agents) {
        /* Check mutual visibility/range. */
        std::string id = a.second->getId();
        bool has_los = hasLineOfSight(a.second);
        bool is_in_range = isInRange(a.second);
        if(has_los && is_in_range) {
            if(!m_connected[id]) {
                m_self_view.setLink(id, AgentLinkView::State::LINE_OF_SIGHT, a.second->getMotion().getPosition());
            }
            if(!m_connected[id] && m_enabled) {
                /* This agent wasn't in range before or we disconnected from it. */
                if(m_encounter_callback(id)) {
                    if(a.second->getLink()->tryConnect(shared_from_this())) {
                        doConnect(id);
                        if(Config::verbosity) {
                            Log::dbg << "Agents connected " << getAgentId() << " <--> " << id << ".\n";
                        }
                    }
                }
            }
        } else {
            m_self_view.setLink(id, AgentLinkView::State::DISCONNECTED);
        }
    }
}

bool AgentLink::hasLineOfSight(const std::shared_ptr<Agent>& aptr)
{
    sf::Vector3f s = m_position;
    sf::Vector3f d = aptr->getMotion().getPosition();

    float s_len  = MathUtils::norm(s);
    float theta  = std::asin(Config::earth_radius / s_len);
    auto ds_norm = MathUtils::makeUnitary(d - s);
    auto s_norm  = MathUtils::makeUnitary(s);
    float adiff  = std::acos(std::fabs(MathUtils::dot(ds_norm, s_norm)));

    float h = std::sqrt(std::pow(s_len, 2.0) - std::pow(Config::earth_radius, 2.0));

    if(adiff > theta) {
        return true;
    } else if(MathUtils::norm(d - s) > h) {
        return false;
    } else {
        return true;
    }
}

bool AgentLink::isInRange(const std::shared_ptr<Agent>& aptr)
{
    float ra = aptr->getLink()->getRange();
    float rb = m_range;
    float d = aptr->getLink()->distanceFrom(m_position);
    return (d < ra && d < rb);
}


std::vector<std::shared_ptr<Activity> > AgentLink::readRxQueue(void)
{
    std::vector<std::shared_ptr<Activity> > rcv_act;
    for(auto& transfer_list : m_rx_queue) {
        for(auto it = transfer_list.second.begin(); it != transfer_list.second.end(); ) {
            if((*it).finished) {
                rcv_act.push_back(((*it).msg));
                it = transfer_list.second.erase(it);
            } else {
                it++;
            }
        }
    }
    return rcv_act;
}

float AgentLink::readEnergyConsumed(void)
{
    float retval = m_energy_consumed;
    m_energy_consumed = 0.f;
    return retval;
}

/* To be called only by the sender AgentLink (to the receiver). */
bool AgentLink::startTransfer(std::string aid, const Transfer& data)
{
    if(!m_enabled) {
        Log::err << "A transfer start has been requested by agent " << aid << " to "
            << getAgentId() << ", but its link is disabled. Will not accept the transfer.\n";
        return false;
    }
    if(!m_connected[aid]) {
        Log::err << "A transfer start has been requested by agent " << aid << " to "
            << getAgentId() << ", but agents are not connected. Will not accept the transfer.\n";
        return false;
    }
    if(data.t_start > VirtualTime::now()) {
        Log::err << "A transfer start has been requested by agent " << aid << " to "
            << getAgentId() << ", but start time is in future. Will not accept the transfer.\n";
        return false;
    }
    if(data.t_start >= data.t_end) {
        Log::err << "A transfer start has been requested by agent " << aid << " to "
            << getAgentId() << ", but end time is wrong. Will not accept the transfer.\n";
        return false;
    }
    m_rx_queue[aid].push_back(data);
    return true;
}

/* To be called only by the sender AgentLink (to the receiver). */
void AgentLink::endTransfer(std::string aid, const Transfer& data)
{
    /* This transfer is in RX queue and should not have been completed: */
    for(auto it = m_rx_queue[aid].begin(); it != m_rx_queue[aid].end(); it++) {
        if(*it == data) {
            if(it->finished) {
                Log::warn << "Agent " << aid << " finished a transfer that had already been completed\n";
            }
            it->finished = true;
            break;
        }
    }
}

/* To be called only by the sender AgentLink (to the receiver). */
void AgentLink::cancelTransfer(std::string aid, const Transfer& data)
{
    /* This transfer is in RX queue and should not have been completed: */
    for(auto it = m_rx_queue[aid].begin(); it != m_rx_queue[aid].end(); it++) {
        if(*it == data) {
            if(it->finished) {
                Log::warn << "Agent " << aid << " requested a transfer cancel that had already been completed\n";
            }
            m_rx_queue[aid].erase(it);
            break;
        }
    }
}

void AgentLink::enable(void)
{
    /*  NOTE: an AgentLink should be enabled/kept enabled (by its owner Agent) iff:
     *  The agent has enough energy to receive, i.e. if the sum of energy consumed by the link and
     *  the energy allocated (not necessarily consumed) by the payload, are lower than the
     *  available capacity. This is to say that the Agent will perform two types of energy
     *  management: (1) predictive through scheduling (which leaves a margin for operations); and
     *  (2) dynamic and not predicted which uses that margin.
     **/
    m_enabled = true;
}

void AgentLink::disable(void)
{
    Log::dbg << "Agent " << getAgentId() << " is going to disconnect from all agents (" << m_link_ranges.size() << " active connections).\n";
    for(auto& a : m_other_agents) {
        if(m_connected[a.first]) {
            doDisconnect(a.second->getId());
            /* Notify of disconnection: */
            m_other_agents[a.second->getId()]->getLink()->notifyDisconnect(getAgentId());
        }
    }
    m_enabled = false;
}

void AgentLink::step(void)
{
    if(m_enabled) {
        /* Start new transfers: */
        double t = VirtualTime::now();
        double tx_duration = VirtualTime::toVirtual(Config::activity_size / m_datarate, TimeValueType::SECONDS);
        for(auto& txq : m_tx_queue) {
            bool sending = false;
            if(txq.second.size() > 0) {
                double next_start = t;
                bool new_tx = false;
                std::vector<unsigned int> clean_txt;
                for(auto& txt : txq.second) {
                    /*  (1a) Start condition: ======================================================
                     *  Prepare start and end times for new transfers.
                     **/
                    if(txt.t_start == -1.0 && !txt.finished) {
                        /* This transfer has not started. Configure its start and end times: */
                        txt.t_start = next_start;
                        next_start += tx_duration;
                        new_tx = true;
                    } else if(txt.t_start != -1.0 && new_tx && !txt.finished) {
                        /* This transfer was configured but has changed because a new one was added before: */
                        if(txt.t_start > t) {
                            txt.t_start = next_start;
                            next_start += tx_duration;
                            Log::warn << "Fixing agent " << getAgentId() << " TX queue for transfer " << txt.id << ".\n";
                        } else {
                            /* The transfer started already. Can't modify it. */
                            Log::err << "Fatal error in agent " << getAgentId() << " TX queue: a new transfer has been added before some started ones.\n";
                            throw std::runtime_error("Unexpected fatal failure in TX queue");
                        }
                    }

                    /*  (1b) Start condition: ======================================================
                     *  Do start those transfers that have to, according to their start time.
                     **/
                    bool start_flag = false;
                    if(txt.t_start <= t && !txt.started) {
                        /* Start the transfer now. */
                        txt.started = true;
                        txt.t_end   = txt.t_start + tx_duration;
                        if(!m_other_agents[txq.first]->getLink()->startTransfer(getAgentId(), txt)) {
                            /* Error, could not be started: */
                            txt.t_start  = -1.f;
                            txt.t_end    = -1.f;
                            txt.finished = true;
                            Log::warn << "Agent " << getAgentId() << " failed to start a transfer with " << txq.first << ".\n";
                        } else {
                            start_flag = true;
                            /*
                            Log::warn << "Agent " << getAgentId() << " started transfer " << txt.id << " with " << txq.first << ". ";
                            Log::warn << "Transfer completes at " << VirtualTime::toString(txt.t_end);
                            Log::warn << " (after " << Config::activity_size / m_datarate << " sec. = " << VirtualTime::toString(tx_duration, false) << ").\n";
                            */
                            sending = true;
                        }
                    }
                    /*  (2) End condition:
                     *  Do end those transfers that have to, according to their end time.
                     **/
                    if(txt.t_end <= t && !txt.finished && txt.started) {
                        /*
                        Log::warn << "Agent " << getAgentId() << " completed transfer " << txt.id << " with " << txq.first
                            << " after " << (txt.t_end - txt.t_start) * 24 * 3600 << " sec.\n";
                        */
                        m_callback_success[txt.id](txt.id);
                        m_other_agents[txq.first]->getLink()->endTransfer(getAgentId(), txt);
                        m_callback_success.erase(m_callback_success.find(txt.id));
                        m_callback_failure.erase(m_callback_failure.find(txt.id));
                        txt.finished = true;

                        if(start_flag) {
                            /* This transfer started and ended in the same time step: */
                            double t_total = txt.t_end - txt.t_start;
                            m_energy_consumed += Config::link_tx_energy_rate * (t_total / Config::time_step);
                        } else {
                            /* Compute the remaining energy consumption for this step: */
                            m_energy_consumed += Config::link_tx_energy_rate * ((Config::time_step - (t - txt.t_end)) / Config::time_step);
                        }

                    /*  (3) Continue condition:
                     *  Compute energy consumption of transfers that are on-going:
                     **/
                    } else if(txt.t_end > t && !txt.finished && txt.started) {
                        /* The transfer is on-going. */
                        sending = true;
                        if(start_flag) {
                            /* This transfer started in this time step: */
                            m_energy_consumed += Config::link_tx_energy_rate * ((t - txt.t_start) / Config::time_step);
                        } else {
                            m_energy_consumed += Config::link_tx_energy_rate;
                        }
                    }
                }

                /* Clean finished transfers: */
                for(auto txtit = txq.second.begin(); txtit != txq.second.end(); ) {
                    if(txtit->finished) {
                        txtit = txq.second.erase(txtit);
                    } else {
                        txtit++;
                    }
                }
            }
            if(sending) {
                m_self_view.setLink(txq.first, AgentLinkView::State::SENDING, m_other_agents[txq.first]->getMotion().getPosition());
            } else if(m_connected[txq.first]) {
                m_self_view.setLink(txq.first, AgentLinkView::State::CONNECTED, m_other_agents[txq.first]->getMotion().getPosition());
            }
        }
        for(auto& rxq : m_rx_queue) {
            for(auto& rxt : rxq.second) {
                if(!rxt.finished && rxt.t_start <= t && rxt.t_end > t && rxt.started) {
                    /* It has not finished: */
                    if(rxt.t_start > t - Config::time_step) {
                        m_energy_consumed += Config::link_rx_energy_rate * ((t - rxt.t_start) / Config::time_step);
                    } else {
                        m_energy_consumed += Config::link_rx_energy_rate;
                    }
                } else if(rxt.t_end > t - Config::time_step) {
                    /*  It has just finished.
                     *  NOTE: the condition (rxt.finished) is not checked in this branch because
                     *      depending on the agent update/step order, some transfers that have
                     *      actually finished may still have this flag unset.
                     **/
                    if(rxt.t_end - rxt.t_start <= Config::time_step) {
                        double t_total = rxt.t_end - rxt.t_start;
                        m_energy_consumed += Config::link_rx_energy_rate * (t_total / Config::time_step);
                    } else {
                        m_energy_consumed += Config::link_rx_energy_rate * ((Config::time_step - (t - rxt.t_end)) / Config::time_step);
                    }
                }
            }
        }
    }
}

std::set<int> AgentLink::listSending(std::string agent_id) const
{
    std::set<int> retset;    /* We build a set to prevent repeated. */
    for(auto& txq : m_tx_queue) {
        for(auto& txt : txq.second) {
            if(txt.msg->getAgentId() == agent_id) {
                retset.insert(txt.msg->getId());
            }
        }
    }
    return retset;
}

float AgentLink::distanceFrom(sf::Vector3f p) const
{
    sf::Vector3f v = p - m_position;
    return MathUtils::norm(v);
}

int AgentLink::scheduleSend(std::shared_ptr<const Activity> a, std::string aid, std::function<void(int)> on_sent, std::function<void(int)> on_failure)
{
    Transfer new_transfer;
    new_transfer.t_start  = -1.0;
    new_transfer.t_end    = -1.0;
    new_transfer.finished = false;
    new_transfer.started  = false;
    new_transfer.id       = m_tx_count;
    new_transfer.msg      = std::make_shared<Activity>(*a); /* Copies the activity. */
    m_tx_queue[aid].push_back(new_transfer);
    m_callback_success[m_tx_count] = on_sent;
    m_callback_failure[m_tx_count] = on_failure;
    m_tx_count++;
    return 0;
}

inline std::string AgentLink::getAgentId(void) const
{
    return m_agent->getId();
}
