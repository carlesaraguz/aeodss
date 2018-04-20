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

struct AgentState;

class Intent
{
public:
    float tstart;
    float tend;
    unsigned int id;

    Intent(float t1, float t2);
    Intent(unsigned int iid, float s, float t1, float t2);
    Intent(unsigned int iid, float s, float t1, float t2, sf::Vector2f p1, sf::Vector2f p2);
    Intent(void);

    bool setPositions(std::map<float, AgentState>::const_iterator it0, std::map<float, AgentState>::const_iterator it1);
    sf::Vector2f getPositionAt(float t) const;
    void setAgentSwath(float s) { m_agent_swath = s; }
    void setPosition(float t, sf::Vector2f p) { m_positions[t] = p; }
    std::size_t getPositionCount(void) const { return m_positions.size(); }

    /* Getters and setters: */
    sf::Vector2f getPStart(void) const { return m_positions.cbegin()->second; }
    sf::Vector2f getPEnd(void) const { return m_positions.crbegin()->second; }
    std::vector<sf::Vector2f> getPositions(void) const;
    void setAgentId(std::string id) { m_agent_id = id; }
    std::string getAgentId(void) const { return m_agent_id; }
    float getAgentSwath(void) const { return m_agent_swath; }

    friend std::ostream& operator<<(std::ostream& os, const Intent& i);

private:
    std::string m_agent_id;
    float m_agent_swath;
    std::map<float, sf::Vector2f> m_positions;

};

#endif /* INTENT_HPP */
