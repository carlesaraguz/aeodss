/***********************************************************************************************//**
 *  A ground station.
 *  @class      GroundStation
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mmm-00
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "GroundStation.hpp"

GroundStation::GroundStation(unsigned int id)
    : GroundStation(id,
        Utils::genRandom(-Constants::pi / 2.0, Constants::pi / 2.0),
        Utils::genRandom(-Constants::pi, Constants::pi),
        Utils::genRandom(0.0, Config::max_gs_altitude))
{ }

GroundStation::GroundStation(unsigned int id, double lat, double lon, double alt)
    : m_latitide(lat)
    , m_longitude(lon)
    , m_altitude(alt)
    , m_gs_id(id)
{
    setDays(0.0);
}

Vector3D GroundStation::setDays(double d)
{
    /*  m_time is expressed in radians.
     *  [rad] = 2π · [rev] = 2π · days.
     */
    m_time = d * 2.0 * Constants::pi;
    m_position.x = std::cos(m_longitude + m_time) * std::cos(m_latitide);
    m_position.y = std::sin(m_longitude + m_time) * std::cos(m_latitide);
    m_position.z = std::sin(m_latitide);
    m_position *= Constants::earth_radius + m_altitude;
    return m_position;
}

bool GroundStation::isVisible(const Satellite& sat) const
{
    Vector3D v = sat.getPosition() - m_position;
    return (v.angle(m_position)) >= (Constants::pi / 2.0);
}
