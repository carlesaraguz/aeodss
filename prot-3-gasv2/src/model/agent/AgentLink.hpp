/***********************************************************************************************//**
 *  The communications device of an agent.
 *  @class      AgentLink
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-24
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef AGENT_LINK_HPP
#define AGENT_LINK_HPP

#include "prot.hpp"
#include "Activity.hpp"

class Agent;

class AgentLink : public TimeStep, public std::enable_shared_from_this<AgentLink>
{
private:
    struct Transfer;    /* Forward declaration. */

public:
    AgentLink(Agent* aptr);
    AgentLink(Agent* aptr, float range);

    /* Getters and setters: */
    float getRange(void) const { return m_range; }
    float getDatarate(void) const { return m_datarate; }
    void setAgents(std::vector<std::shared_ptr<Agent> > agents);
    inline std::string getAgentId(void) const;
    void setPosition(sf::Vector2f p) { m_position = p; }
    void setEncounterCallback(std::function<bool(std::string)> f) { m_encounter_callback = f; }

    /* Time step and link control: */
    void update(void);
    void step(void) override;

    /* Connection management and requests: */
    bool tryConnect(std::shared_ptr<AgentLink> other);
    void notifyDisconnect(std::string aid_other);

    /* Interface for Agent: */
    std::vector<Activity> readRxQueue(void);
    float readEnergyConsumed(void);
    int scheduleSend(Activity a, std::string aid,
        std::function<void(int)> on_sent = [](int) { },
        std::function<void(int)> on_failure = [](int) { });
    void enable(void);
    void disable(void);

private:
    struct Transfer {
        std::shared_ptr<Activity> msg;
        float t_start;
        float t_end;
        bool finished;
        unsigned int id;

        bool operator==(const Transfer& other) {
            return other.id == id;
        }
    };

    sf::Vector2f m_position;
    bool m_enabled;
    float m_range;
    float m_datarate;
    float m_energy_consumed;
    int m_tx_count;
    Agent* m_agent;
    std::function<bool(std::string)> m_encounter_callback;
    std::map<std::string, std::shared_ptr<Agent> > m_other_agents;  /* Stays constant throughout execution. */
    std::map<std::string, bool> m_connected;
    std::map<std::string, float> m_link_ranges;                     /* Constantly updated and changed. */
    std::map<std::string, std::vector<Transfer> > m_tx_queue;
    std::map<std::string, std::vector<Transfer> > m_rx_queue;
    std::map<int, std::function<void(int)> > m_callback_success;
    std::map<int, std::function<void(int)> > m_callback_failure;

    /* Connection management: */
    void doConnect(std::string aid);
    void doDisconnect(std::string aid);

    /* Internal transfer control: */
    bool startTransfer(std::string aid, const Transfer& data);
    void cancelTransfer(std::string aid, const Transfer& data);
    void endTransfer(std::string aid, const Transfer& data);
    float distanceFrom(sf::Vector2f p) const;
};

#endif /* AGENT_LINK_HPP */
