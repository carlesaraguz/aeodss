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
#include "Config.hpp"

class VirtualTime
{
public:
    static void step(void);
    static std::string toString(double t = -1.0, bool is_absolute_time = true);
    static double now(void) { return m_vtime; }
    static void init(double t);
    static bool isInit(void) { return m_initialized; }

private:
    static bool m_initialized;
    static double m_vtime;
};

#endif /* VIRTUAL_TIME_HPP */
