/***********************************************************************************************//**
 *  A single information unit about agent intents.
 *  @class      Intent
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-22
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef INTENT_HPP
#define INTENT_HPP

#include "prot.hpp"

class Intent
{
public:
    float tstart;
    float tend;
    sf::Vector2f pstart;
    sf::Vector2f pend;
    unsigned int id;

    Intent(unsigned int iid, float t1, float t2, sf::Vector2f p1 = {0.f, 0.f}, sf::Vector2f p2 = {0.f, 0.f});
    Intent(unsigned int iid, sf::Vector2f p1 = {0.f, 0.f}, sf::Vector2f p2 = {0.f, 0.f});
    Intent(void);

    void setAgentId(std::string id) { m_agent_id = id; }
    std::string getAgentId(void) const { return m_agent_id; }

private:
    std::string m_agent_id;
};

#endif /* INTENT_HPP */
