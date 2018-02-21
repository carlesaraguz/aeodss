/***********************************************************************************************//**
 *  Program configuration values.
 *  @class      Config
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "prot.hpp"

class Config
{
public:
    static constexpr const double max_gs_altitude = 500;    /**< Max. GS altitude in meters.    */
    static constexpr const double max_leo_altitude = 900e3; /**< Max. orbital height in meters. */
    static constexpr const double min_leo_altitude = 500e3; /**< Min. ornital height in meters. */
    static double max_time;                                 /**< Simulation time, in seconds.   */
    static double time_step;                                /**< Simulation step, in seconds.   */
    static int n_sats;                                      /**< Number of satellites.          */
};


#endif /* CONFIG_HPP */
