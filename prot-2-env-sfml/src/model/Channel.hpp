/***********************************************************************************************//**
 *  Transmission medium where Agents can communicate to one another.
 *  @class      Channel
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-05
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "prot.hpp"
#include "Agent.hpp"

class Channel
{
public:
    Channel(void) = default;
    Channel(const std::vector<std::shared_ptr<Agent> >& agents);

    void addAgent(std::shared_ptr<Agent> agent);
    void handleLinks(void);
    void configAgents(void) const;

private:
    std::vector<std::shared_ptr<Agent> > m_agents_list;

};

#endif /* CHANNEL_HPP */
