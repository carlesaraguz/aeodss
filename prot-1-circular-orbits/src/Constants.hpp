/***********************************************************************************************//**
 *  Program constant values.
 *  @class      Constants
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include "prot.hpp"

class Constants
{
public:
    static constexpr const double pi             = std::acos(-1);   /* Pi.                        */
    static constexpr const double earth_radius   = 6371.0e3;        /* Earth radius in meters.    */
    static constexpr const double light_speed    = 3e8;             /* Speed of light in m/s².    */
    static constexpr const double earth_mu       = 3.986004418e14;  /* Earth's grav. parameter in m³/s². */
};


#endif /* CONSTANTS_HPP */
