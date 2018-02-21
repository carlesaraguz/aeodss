/***********************************************************************************************//**
 *  Description.
 *  @class      Satellite
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mmm-00
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef SATELLITE_HPP
#define SATELLITE_HPP

#include "prot.hpp"

#include "Utils.hpp"
#include "Vector3D.hpp"

class Satellite
{
public:
    /*******************************************************************************************//**
     *  Default constructor.
     **********************************************************************************************/
    Satellite(unsigned int id);
    Satellite(unsigned int id, double r, double r1, double r2);

    /*******************************************************************************************//**
     *  Auto-generated default destructor.
     **********************************************************************************************/
    ~Satellite(void) = default;

    Satellite(const Satellite&) = default;
    Satellite& operator=(const Satellite&) = default;
    Satellite(Satellite&&) = default;
    Satellite& operator=(Satellite&&) = default;

    Vector3D setDays(double d);
    Vector3D getPosition(void) const { return m_position; }

private:
    struct Orbit {
        double radius;
        double r1;      /* Rotation 1 (radians).        */
        double r2;      /* Rotation 2 (radians).        */
        Vector3D u;     /* Vector u.                    */
        Vector3D nxu;   /* Vetcor n x u.                */
        double period;  /* Orbital period (in seconds). */
    } m_orbit;
    Vector3D m_position;
    double m_time;
    unsigned int m_sat_id;
};

#endif /* SATELLITE_HPP */
