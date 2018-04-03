/***********************************************************************************************//**
 *  Description.
 *  @class      Satellite
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mmm-00
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Satellite.hpp"

Satellite::Satellite(unsigned int id)
    : Satellite(id,
        Utils::genRandom(Config::min_leo_altitude, Config::max_leo_altitude) + Constants::earth_radius,
        Utils::genRandom(0, 2 * Constants::pi),
        Utils::genRandom(0, 2 * Constants::pi))
{ }

Satellite::Satellite(unsigned int id, double r, double r1, double r2)
    : m_orbit({r, r1, r2, Vector3D(), Vector3D(), 0.0})
    , m_sat_id(id)
{
    Vector3D u(-std::sin(m_orbit.r1), std::cos(m_orbit.r1), 0.0);
    Vector3D nxu(
        std::cos(m_orbit.r1) * std::cos(m_orbit.r2),
        std::sin(m_orbit.r1) * std::cos(m_orbit.r2),
        -std::cos(m_orbit.r2)
    );
    m_orbit.u = u;
    m_orbit.nxu = nxu;
    m_orbit.period = 2 * Constants::pi * std::sqrt((r * r * r) / Constants::earth_mu);
    setDays(0.0);

    #if 0
    /* Debug messages: */
    std::cout << std::scientific << std::setprecision(4);
    std::cout << std::setw(6) << std::setfill(' ')
              << "Satellite created: " << m_sat_id << " "
              << "[r " << r << "] "
              << "[r1 " << r1 << "] "
              << "[r2 " << r2 << "] "
              << "[T " << (int)(m_orbit.period / 60) << " min. ]";
    std::cout << std::defaultfloat << std::endl;
    #endif
}

Vector3D Satellite::setDays(double d)
{
    /*  m_time is expressed in radians.
     *  [rad] = 2π · [rev]
     *  [rev] = days · [rev/day]
     *  [rev/day] = [rev/s] · [s/day] = f · (24·60·60) / 1 = 24·60·60 / T
     **/
    m_time = 2.0 * Constants::pi * d * (24.0 * 3600.0) / m_orbit.period;
    m_position  = m_orbit.u * m_orbit.radius * std::cos(m_time);
    m_position += m_orbit.nxu * m_orbit.radius * std::sin(m_time);
    return m_position;
}
