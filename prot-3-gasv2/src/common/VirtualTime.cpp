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

std::string VirtualTime::toString(double t, bool is_absolute_time)
{
    std::stringstream ss;
    std::string retval;

    if(t == -1.0) {
        t = m_vtime;
        is_absolute_time = true;
    }

    if(Config::time_type == TimeValueType::JULIAN_DAYS) {
        if(is_absolute_time) {
            t -= Config::start_epoch;
        }
        double sec = std::fmod(t * 60.0 * 60.0 * 24.0, 60.0);
        int min  = (int)(t * 60 * 24) % 60;
        int hour = (int)(t * 24) % 24;
        int days = (int)t;
        ss << "\'" << days << "d"
            << std::setw(2) << std::setfill('0') << hour << ":"
            << std::setw(2) << std::setfill('0') << min  << ":"
            << std::fixed << std::setprecision(3) << std::setw(6) << std::setfill('0') << sec << "\'";
    } else {
        ss << std::fixed << std::setw(3) << m_vtime;
    }
    retval = ss.str();
    return retval;
}
