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
#include "MathUtils.hpp"
#include "Activity.hpp"
#include "VirtualTime.hpp"
#include "AgentLinkView.hpp"

class Agent;

class AgentLink : public TimeStep, public HasView, public std::enable_shared_from_this<AgentLink>
{
private:
    struct Transfer;    /* Forward declaration. */

public:
    /*******************************************************************************************//**
     *  Constructs an AgentLink with a pointer to its owner Agent. Communications range is randomly
     *  generated and is in the range [Config::agent_range_min, Config::agent_range_max].
     *  Completes initialization by calling the complete constructor.
     *  @param  aptr    Pointer to the agent that owns this AgentLink object.
     *  @see AgentLink(Agent*, float)
     **********************************************************************************************/
    AgentLink(Agent* aptr);

    /*******************************************************************************************//**
     *  Constructs an AgentLink with a given communications range (expressed in meters for
     *  AgentMotionType::ORBITAL and pixels otherwise.) Agent datarate is randomly set and is in the
     *  range [Config::agent_datarate_min, Config::agent_datarate_max].
     *  @param  aptr    Pointer to the agent that owns this AgentLink object.
     *  @param  range   Range in meters (for AgentMotionType::ORBITAL) or pixels (all other cases).
     **********************************************************************************************/
    AgentLink(Agent* aptr, float range);

    /*******************************************************************************************//**
     *  Constructs an AgentLink providing all values. No random prameter is set.
     **********************************************************************************************/
    AgentLink(Agent* aptr, float range, float datarate);

    /*******************************************************************************************//**
     *  Getter for the actual range of this link.
     **********************************************************************************************/
    float getRange(void) const { return m_range; }

    /*******************************************************************************************//**
     *  Getter for the datarate of this link.
     **********************************************************************************************/
    float getDatarate(void) const { return m_datarate; }

    /*******************************************************************************************//**
     *  Defines references to all the other existing agents.
     **********************************************************************************************/
    void setAgents(std::vector<std::shared_ptr<Agent> > agents);

    /*******************************************************************************************//**
     *  Gets the agent ID of this AgentLink.
     **********************************************************************************************/
    inline std::string getAgentId(void) const;

    /*******************************************************************************************//**
     *  Sets the current position of this agent in 2d space. Internally, this class uses 3d vectors;
     *  `z` coordinate is set to 0 in this case.
     *  @note   To be used when Config::motion_model is not equal to AgentMotionType::ORBITAL.
     **********************************************************************************************/
    void setPosition(sf::Vector2f p);

    /*******************************************************************************************//**
     *  Sets the current position of this agent in 3d space.
     *  @note   To be used when Config::motion_model is equal to AgentMotionType::ORBITAL.
     **********************************************************************************************/
    void setPosition(sf::Vector3f p);

    /*******************************************************************************************//**
     *  Installs a callback function that will be invoked every time the agent encounters a peer.
     *  The callback function accepts a single string argument with the Agent ID of the encountered
     *  agent. The callbak will only be called if the link is enabled. If the callback function
     *  returns true, the Agent will proceed with the connection. If the agent callback returns
     *  false, neither of the Agents will be connected to the other.
     *  @param  f   Function to be called at the start of an encounter.
     **********************************************************************************************/
    void setEncounterCallback(std::function<bool(std::string)> f) { m_encounter_callback = f; }

    /*******************************************************************************************//**
     *  Installs a callback function that will be invoked every time the agent connects to a peer.
     *  The callback function accepts a single string argument with the Agent ID of the connected
     *  agent. The callbak will only be called if the link is enabled and the encounter has been
     *  previously accepted with the encounter callback.
     *  @param  f   Function to be called once the agents are connected.
     **********************************************************************************************/
    void setConnectedCallback(std::function<void(std::string)> f) { m_connected_callback = f; }

    /*******************************************************************************************//**
     *  Updates links with other agents. Tries to connect to agents in close vicinity and
     *  disconnects from those that are further from their communications range. Connections are
     *  only established if the link is enabled and if the encounter callback returns true. Upon
     *  diconnection, all active transfers to the disconnecting agent are cancelled (also notifying
     *  the other end).
     **********************************************************************************************/
    void update(void);

    /*******************************************************************************************//**
     *  Advances active transfers. Transfers are iterated only if the agent link is enabled.
     *  Iterating over active transfers consumes energy, but this energy is only reflected upon in
     *  an internal counter that can be read and cleared with AgentLink::readEnergyConsumed.
     *  For each Agent, only one transfer (Tx) and one reception (Rx) can occur simultaneously. That
     *  is: agents can send to multiple peers at the same time but each peer queue is a FIFO.
     **********************************************************************************************/
    void step(void) override;

    /*******************************************************************************************//**
     *  Requests a connection to the agent other. This function triggers a call to the encounter
     *  callback for the agent other.
     *  @param  other       The agent ID to which this link has to be connected.
     **********************************************************************************************/
    bool tryConnect(std::shared_ptr<AgentLink> other);

    /*******************************************************************************************//**
     *  Notify of the irrevocable disconnection from aid_other. This function calls doDisconnect.
     *  @param  aid_other   The agent ID from which this link has to be disconnected.
     **********************************************************************************************/
    void notifyDisconnect(std::string aid_other);

    /*******************************************************************************************//**
     *  Dump the Rx queue, effectively clearing any previously finished transfer. Transfers that are
     *  not completed are not dumped.
     **********************************************************************************************/
    std::vector<std::shared_ptr<Activity> > readRxQueue(void);

    /*******************************************************************************************//**
     *  Retrieve and clear the accumulated energy consumption for this link. Once read, the
     *  accumulated value is set back to 0 (and will continue to increment in each step).
     **********************************************************************************************/
    float readEnergyConsumed(void);

    /*******************************************************************************************//**
     *  Adds a new activity to the transfer queue of an agent peer.
     *  @param  a           The activity to send to the other agent.
     *  @param  aid         The agent to send the activity to.
     *  @param  on_sent     A callback that will be invoked once the transfer is successfully
     *                      completed. The int argument identifies the transfer.
     *  @param  on_failure  A callback that will be invoked if the transfer is cancelled. The int
     *                      argument identifies the transfer.
     *  @return An ID of this transfer.
     **********************************************************************************************/
    int scheduleSend(std::shared_ptr<const Activity> a, std::string aid,
        std::function<void(int)> on_sent = [](int) { },
        std::function<void(int)> on_failure = [](int) { });

    /*******************************************************************************************//**
     *  Enable this link.
     **********************************************************************************************/
    void enable(void);

    /*******************************************************************************************//**
     *  Disable this link. Disconnects from all agents, thus cancelling all active transfers and
     *  stopping the consumption of energy altogether.
     **********************************************************************************************/
    void disable(void);

    /*******************************************************************************************//**
     *  Whether the link is in enabled state or not.
     **********************************************************************************************/
    bool isEnabled(void) const { return m_enabled; }

    /*******************************************************************************************//**
     *  Returns a reference to the visual representation of this link.
     **********************************************************************************************/
    const sf::Drawable& getView(void) const { return m_self_view; }

    /*******************************************************************************************//**
     *  Report which activities (their ID's) are being sent that belong to `agent_id`.
     **********************************************************************************************/
    std::set<int> listSending(std::string agent_id) const;

private:
    struct Transfer {
        std::shared_ptr<Activity> msg;  /**< The message to send. */
        double t_start;                 /**< Start time of the transfer. */
        double t_end;                   /**< Potential end time of the transfer (unless cancelled). */
        bool finished;                  /**< Completion flag (true = has completed). */
        bool started;                   /**< Start flag (true = has started). */
        unsigned int id;                /**< Transfer ID. */

        bool operator==(const Transfer& other) {
            return other.id == id;
        }
    };

    sf::Vector3f m_position;            /**< The current position of this agent (used to compute ranges). */
    bool m_enabled;                     /**< Whether the link is enabled or not. */
    float m_range;                      /**< Maximum range for this communication link. */
    float m_datarate;                   /**< Maximum datarate for this communication link. */
    float m_energy_consumed;            /**< The accumulated energy consumption. */
    int m_tx_count;                     /**< Internal counter of transfers (used to generated transfer ID's). */
    Agent* m_agent;                     /**< Pointer to the owning agent object. */
    std::function<bool(std::string)> m_encounter_callback;          /**< Invoked when one agent is in range with another. */
    std::function<void(std::string)> m_connected_callback;          /**< Invoked when one agent is connected to another. */
    std::map<std::string, std::shared_ptr<Agent> > m_other_agents;  /**< Stays constant throughout execution. */
    std::map<std::string, bool> m_connected;                        /**< Connection look up table. */
    std::map<std::string, float> m_link_ranges;                     /**< Constantly updated and changed. */
    std::map<std::string, std::vector<Transfer> > m_tx_queue;       /**< Transfer queue to each connected agent. */
    std::map<std::string, std::vector<Transfer> > m_rx_queue;       /**< Reception queue for each conencted agent. */
    std::map<std::string, double> m_reconnect_time;                 /**< Values of reconnection for agents whose transfer queue is empty. */
    std::map<int, std::function<void(int)> > m_callback_success;    /**< Callbacks for each transfer (on success). */
    std::map<int, std::function<void(int)> > m_callback_failure;    /**< Callbacks for each transfer (on cancellation). */
    AgentLinkView m_self_view;

    /*******************************************************************************************//**
     *  Establishes a connection with the agent aid.
     **********************************************************************************************/
    void doConnect(std::string aid);

    /*******************************************************************************************//**
     *  Disconnects from an agent to which it was previously connected.
     **********************************************************************************************/
    void doDisconnect(std::string aid);

    /*******************************************************************************************//**
     *  Starts a new transfer. Registers the transfer in the RX queue.
     *  @return True if the transfer is successfully started, false otherwise.
     *  @note   To be called only by the sender AgentLink (to the receiver).
     **********************************************************************************************/
    bool startTransfer(std::string aid, const Transfer& data);

    /*******************************************************************************************//**
     *  Cancels an on-going transfer. Removes the active transfer from the internal RX queue.
     *  @note   To be called only by the sender AgentLink (to the receiver).
     **********************************************************************************************/
    void cancelTransfer(std::string aid, const Transfer& data);

    /*******************************************************************************************//**
     *  Cancels an on-going transfer. Keeps transfer in the internal RX queue and sets its finished
     *  flag.
     *  @note   To be called only by the sender AgentLink (to the receiver).
     **********************************************************************************************/
    void endTransfer(std::string aid, const Transfer& data);

    /*******************************************************************************************//**
     *  Cleans a queue by removing all the completed transfers.
     **********************************************************************************************/
    void cleanFinishedQueue(std::vector<Transfer>& queue);

    /*******************************************************************************************//**
     *  Compute the distance to an agent located at p.
     *  @param  p   Position to which the distance has to be computed.
     *  @return     The value in the corresponding distance units (i.e. meters or pixels).
     **********************************************************************************************/
    float distanceFrom(sf::Vector3f p) const;

    /*******************************************************************************************//**
     *  Determines whether agent aptr is in line of sight.
     *  @param  aptr    Pointer to the other agent.
     **********************************************************************************************/
    bool hasLineOfSight(const std::shared_ptr<Agent>& aptr);

    /*******************************************************************************************//**
     *  Determines whether agent aptr is within mutual link ranges.
     *  @param  aptr    Pointer to the other agent.
     **********************************************************************************************/
    bool isInRange(const std::shared_ptr<Agent>& aptr);

    /*******************************************************************************************//**
     *  Estimates the duration of a transfer (best-case scenario). The time is computed as B/D,
     *  where B is the size of the activity in bytes, and D is the datarate in bytes per second.
     *  Activity sizes are determined with the number of points in their trajectory plus a fixed
     *  size defined in the configuration file. Datarate is taken in bits per second (bps) and
     *  transformed to bytes per second. Time is returned in virtual units (i.e. it can be used
     *  directly).
     *  @param  msg     The activity that will be transferred.
     *  @param  dr      Datarate in bits per second.
     **********************************************************************************************/
    double getTxTime(std::shared_ptr<Activity> msg, float dr) const;

    /*******************************************************************************************//**
     *  Performs a partial step: detects newly scheduled transfers and prepares them.
     *  @param  t           The current virtual time.
     *  @param  aid         Agent to which txt is being sent to.
     *  @param  txt         The transfer object to step.
     *  @param  new_tx      A flag that will be set to true if new transfers have been detected.
     *                      Else its value will not be changed.
     *  @param  next_start  Accumulator of time used to concatenate transfers.
     **********************************************************************************************/
    void step1a(double t, std::string aid, Transfer& txt, bool& new_tx, double& next_start);

    /*******************************************************************************************//**
     *  Performs a partial step: starts transfers that have to be started (or had to be started int
     *  the past).
     *  @param  t           The current virtual time.
     *  @param  aid         Agent to which txt is being sent to.
     *  @param  txt         The transfer object to step.
     *  @param  sending     A flag that will be set to true if agent is currently transferring data.
     *  @return             True if the transfer has been started. False otherwise.
     **********************************************************************************************/
    bool step1b(double t, std::string aid, Transfer& txt, bool& sending);

    /*******************************************************************************************//**
     *  Performs a partial step: (1) ends transfers that have to end; and (2) identifies transfers
     *  that still need to continue.
     *  @param  t           The current virtual time.
     *  @param  aid         Agent to which txt is being sent to.
     *  @param  txt         The transfer object to step.
     *  @param  start_flag  The value returned by step1 (i.e. true if some transfer was started,
     *                      false otherwise).
     *  @param  sending     A flag that will be set to true if agent is currently transferring data.
     **********************************************************************************************/
    void step2(double t, std::string aid, Transfer& txt, bool start_flag, bool& sending);

    /*******************************************************************************************//**
     *  Executes step 1b and 2, and calls cleanFinishedQueue.
     **********************************************************************************************/
    void doPartialStep(std::string aid);
};

#include "Agent.hpp"

#endif /* AGENT_LINK_HPP */
