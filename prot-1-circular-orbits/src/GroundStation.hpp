/***********************************************************************************************//**
 *  A ground station.
 *  @class      GroundStation
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mmm-00
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef GROUND_STATION_HPP
#define GROUND_STATION_HPP

#include "prot.hpp"

#include "Utils.hpp"
#include "Vector3D.hpp"
#include "Satellite.hpp"

class GroundStation
{
public:
    /*******************************************************************************************//**
     *  Default constructor.
     **********************************************************************************************/
    GroundStation(unsigned int id);
    GroundStation(unsigned int id, double lat, double lon, double alt);

    /*******************************************************************************************//**
     *  Auto-generated default destructor.
     **********************************************************************************************/
    ~GroundStation(void) = default;

    GroundStation(const GroundStation&) = default;
    GroundStation& operator=(const GroundStation&) = default;
    GroundStation(GroundStation&&) = default;
    GroundStation& operator=(GroundStation&&) = default;

    Vector3D setDays(double d);
    bool isVisible(const Satellite& sat) const;

private:
    Vector3D m_position;
    double m_time;
    unsigned int m_gs_id;
    double m_latitide;
    double m_longitude;
    double m_altitude;
};

#endif /* GROUND_STATION_HPP */
