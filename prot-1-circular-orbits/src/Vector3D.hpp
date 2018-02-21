/***********************************************************************************************//**
 *  A 3-dimensional vector.
 *  @class      Vector3D
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef VECTOR_3D_HPP
#define VECTOR_3D_HPP

#include "prot.hpp"

class Vector3D
{
public:
    double x;
    double y;
    double z;

    Vector3D(void);
    ~Vector3D(void) = default;

    Vector3D(double rx, double ry, double rz);

    Vector3D(const Vector3D&) = default;
    Vector3D& operator=(const Vector3D&) = default;
    Vector3D(Vector3D&&) = default;
    Vector3D& operator=(Vector3D&&) = default;

    /*******************************************************************************************//**
     *  Computes the modulo of the vector.
     **********************************************************************************************/
    double modulo(void) const;

    /*******************************************************************************************//**
     *  Returns a normalized version of this vector.
     **********************************************************************************************/
    Vector3D getNorm(void) const;

    /*******************************************************************************************//**
     *  Normalizes this vector.
     **********************************************************************************************/
    void normalize(void);

    /*******************************************************************************************//**
     *  Computes the lenght of the difference vector (i.e. the distance between two vectors.)
     **********************************************************************************************/
    double distance(const Vector3D& rval) const;

    /*******************************************************************************************//**
     *  Computes the dot product and returns the angle between two vectors.
     **********************************************************************************************/
    double angle(const Vector3D& rval) const;

    Vector3D operator+(const Vector3D& rval) const;
    Vector3D operator-(const Vector3D& rval) const;
    Vector3D operator*(const Vector3D& rval) const;
    Vector3D operator*(const double& k) const;
    Vector3D operator/(const double& k) const;
    Vector3D& operator+=(const Vector3D& rval);
    Vector3D& operator-=(const Vector3D& rval);
    Vector3D& operator*=(const Vector3D& rval);
    Vector3D& operator*=(const double& k);
    Vector3D& operator/=(const double& k);
    bool operator==(const Vector3D& rval) const;
    bool operator!=(const Vector3D& rval) const;
    bool operator<=(const Vector3D& rval) const;
    bool operator>=(const Vector3D& rval) const;
    bool operator<(const Vector3D& rval) const;
    bool operator>(const Vector3D& rval) const;
};

#endif /* VECTOR_3D_HPP */
