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

CREATE_LOGGER(VirtualTime)

double VirtualTime::m_vtime = Config::start_epoch;

void VirtualTime::step(void)
{
    m_vtime += Config::time_step;
}

std::string VirtualTime::toString(double t)
{
    std::stringstream ss;
    std::string retval;

    if(t == -1.0) {
        t = m_vtime;
    }
    if(Config::motion_model == AgentMotionType::ORBITAL) {
        std::stringstream ss;
        t -= Config::start_epoch;
        int sec  = (int)(t * 60 * 60 * 24) % 60;
        int min  = (int)(t * 60 * 24) % 60;
        int hour = (int)(t * 24) % 24;
        int days = (int)t;
        ss << days << "d "
            << std::setw(2) << std::setfill('0') << hour << ":"
            << std::setw(2) << std::setfill('0') << min  << ":"
            << std::setw(2) << std::setfill('0') << sec;
    } else {
        ss << m_vtime;
    }
    retval = ss.str();
    return retval;
}
