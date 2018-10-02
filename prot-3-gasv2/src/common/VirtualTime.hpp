/***********************************************************************************************//**
 *  Global virtual time.
 *  @class      VirtualTime
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-19
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef VIRTUAL_TIME_HPP
#define VIRTUAL_TIME_HPP

#include "prot.hpp"

class VirtualTime
{
public:
    static void step(void);
    static std::string getTimeStr(void);
    static float now(void) { return m_vtime; }
private:
    static float m_vtime;
};

#endif /* VIRTUAL_TIME_HPP */
