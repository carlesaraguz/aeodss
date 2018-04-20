/***********************************************************************************************//**
 *  A single information unit about agent intents.
 *  @class      Intent
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-22
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Intent.hpp"
#include "Agent.hpp"

Intent::Intent(float t1, float t2)
    : Intent(0, 0.f, t1, t2)
{ }

Intent::Intent(unsigned int iid, float s, float t1, float t2)
    : tstart(t1)
    , tend(t2)
    , m_agent_swath(s)
    , id(iid)
{ }

Intent::Intent(unsigned int iid, float s, float t1, float t2, sf::Vector2f p1, sf::Vector2f p2)
    : Intent(iid, s, t1, t2)
{
    setPosition(t1, p1);
    setPosition(t2, p2);
}

Intent::Intent::Intent(void)
    : Intent(0, 0.f, 0.f, 0.f)
{ }

bool Intent::setPositions(std::map<float, AgentState>::const_iterator it0, std::map<float, AgentState>::const_iterator it1)
{
    if(std::distance(it0, it1) < 1) {
        std::cerr << "Intent error: can't extract positions from iterators with distance smaller than 2.\n";
        return false;
    }

    m_positions.clear();
    m_positions[it0->first] = it0->second.position;

    sf::Vector2f p = it0->second.position;
    sf::Vector2f v = it0->second.velocity;
    float t = it0->first;
    int aux = 0;
    for(auto it = it0; it != std::prev(it1); it++) {
        if(v != it->second.velocity) {
            /* Velocity changed: the agent bounced. */
            m_positions[t] = p;                             /* Save the previous point (before bounce.) */
            m_positions[it->first] = it->second.position;   /* Save the current point (after bounce.)   */
        }
        p = it->second.position;
        v = it->second.velocity;
        t = it->first;
        aux++;
    }
    if(v != std::prev(it1)->second.velocity) {
        /* The last point was a bounce: */
        m_positions[t] = v;     /* Save the previous point (before bounce.) */
    }
    m_positions[it1->first] = it1->second.position;

    return true;
}

sf::Vector2f Intent::getPositionAt(float t) const
{
    if(m_positions.size() == 0) {
        std::cerr << "Intent [" << id << "] does not have recorded positions.\n";
        return {0.f, 0.f};
    }

    if(t <= tstart) {
        return getPStart();
    } else if(t >= tend) {
        return getPEnd();
    } else {
        auto it0 = m_positions.upper_bound(t);
        auto it1 = std::prev(it0);  /* Since t is not tstart, there must be a previous value. */

        float duration = it1->first - it0->first;
        float progress = (t - it0->first) / duration;

        sf::Vector2f trajectory = it1->second - it0->second;
        return it0->second + trajectory * progress;
    }
}

std::vector<sf::Vector2f> Intent::getPositions(void) const
{
    std::vector<sf::Vector2f> retvec;
    for(const auto& p : m_positions) {
        retvec.push_back(p.second);
    }
    return retvec;
}


std::ostream& operator<<(std::ostream& os, const Intent& i)
{
    os << "Intent " << i.id << ", interval [" << i.tstart << ", " << i.tend << "). Positions:\n";
    for(const auto& p : i.m_positions) {
        os << "-- t(" << p.first << ") => p(" << p.second.x << ", " << p.second.y << ")\n";
    }
    return os;
}
