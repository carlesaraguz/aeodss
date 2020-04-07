/***********************************************************************************************//**
 *  Global wall time.
 *  @class      WallTime
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-19
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef WALL_TIME_HPP
#define WALL_TIME_HPP

#include "prot.hpp"

class WallTime
{
public:
    static std::string getTimeStr(void);
    static unsigned int getSeconds(void);
private:
    static std::time_t m_wall_start_time;
};

#endif /* WALL_TIME_HPP */
