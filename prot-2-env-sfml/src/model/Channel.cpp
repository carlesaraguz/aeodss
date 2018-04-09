/***********************************************************************************************//**
 *  Transmission medium where Agents can communicate to one another.
 *  @class      Channel
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-05
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Channel.hpp"


Channel::Channel(const std::vector<std::shared_ptr<Agent> >& agents)
    : m_agents_list(agents)
{ }

void Channel::addAgent(std::shared_ptr<Agent> agent)
{
    m_agents_list.push_back(agent);
}

void Channel::handleLinks(void)
{
    std::vector<std::string> toggle_links;
    for(auto& agent : m_agents_list) {
        toggle_links.clear();
        auto& ltable = agent->getLinks();

        /* Check which Agents are no longer visible: */
        for(auto& neighbour : ltable[true]) {
            if(!agent->isVisible(neighbour.second)) {
                toggle_links.push_back(neighbour.first);
            }
        }
        /* Check which Agents have become visible: */
        for(auto& neighbour : ltable[false]) {
            if(agent->isVisible(neighbour.second)) {
                toggle_links.push_back(neighbour.first);
            }
        }
        /* Toggle links: */
        for(auto& t : toggle_links) {
            agent->toggleAgentLink(t);
        }
    }
    for(auto& agent : m_agents_list) {
        agent->doCommunicate();
    }
}

void Channel::configAgents(void) const
{
    std::map<bool, std::vector<std::shared_ptr<Agent> > > all_agents;
    all_agents[false] = m_agents_list;
    for(auto& a : m_agents_list) {
        a->addAgentLink(all_agents);
    }
}
