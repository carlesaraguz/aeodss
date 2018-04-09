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

Intent::Intent(unsigned int iid, float s, float t1, float t2, sf::Vector2f p1, sf::Vector2f p2)
    : tstart(t1)
    , pstart(p1)
    , tend(t2)
    , pend(p2)
    , id(iid)
    , m_agent_swath(s)
{ }

Intent::Intent(unsigned int iid, float s, sf::Vector2f p1, sf::Vector2f p2)
    : Intent(iid, s, 0.f, 0.f, p1, p2)
{ }

Intent::Intent(float swath)
    : Intent(0, swath)
{ }

Intent::Intent(void)
    : Intent(0, 0.f)
{ }
