/***********************************************************************************************//**
 *  Global wall time.
 *  @class      WallTime
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-19
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "WallTime.hpp"

std::time_t WallTime::m_wall_start_time = std::time(nullptr);

std::string WallTime::getTimeStr(void)
{
    std::time_t diff = std::time(nullptr) - m_wall_start_time;
    std::stringstream ss;
    std::string str_cpy;
    int sec  = diff % 60;
    int min  = (diff / 60) % 60;
    int hour = (diff / 3600);
    ss << std::setw(2) << std::setfill('0') << hour << ":";
    ss << std::setw(2) << std::setfill('0') << min << ":";
    ss << std::setw(2) << std::setfill('0') << sec;
    str_cpy = ss.str();
    return str_cpy;
}

unsigned int WallTime::getSeconds(void)
{
    return std::time(nullptr) - m_wall_start_time;
}
