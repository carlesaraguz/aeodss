/***********************************************************************************************//**
 *  Global virtual time.
 *  @class      VirtualTime
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-19
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "VirtualTime.hpp"

float VirtualTime::m_vtime = 0.f;

void VirtualTime::step(void)
{
    m_vtime += Config::time_step;
}

std::string VirtualTime::getTimeStr(void)
{
    std::stringstream ss;
    ss << m_vtime;
    std::string retval = ss.str();
    return retval;
}
