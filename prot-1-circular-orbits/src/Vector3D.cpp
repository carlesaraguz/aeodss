/***********************************************************************************************//**
 *  A 3-dimensional vector.
 *  @class      Vector3D
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Vector3D.hpp"

Vector3D::Vector3D(void)
    : Vector3D(0.0, 0.0, 0.0)
{ }

Vector3D::Vector3D(double rx, double ry, double rz)
    : x(rx)
    , y(ry)
    , z(rz)
{ }

double Vector3D::modulo(void) const
{
    return std::sqrt(x * x + y * y + z * z);
}

Vector3D Vector3D::getNorm(void) const
{
    Vector3D v(x, y, z);    /* Creates a temporal copy of this vector. */
    v /= v.modulo();
    return v;
}

void Vector3D::normalize(void)
{
    double mod = modulo();
    x /= mod;
    y /= mod;
    z /= mod;
}

double Vector3D::distance(const Vector3D& rval) const
{
    /* Computes the distance: */
    Vector3D v = *this - rval;
    return v.modulo();
}

double Vector3D::angle(const Vector3D& rval) const
{
    /* Computes dot product and returns the angle in radians: */
    double dotproduct = x * rval.x + y * rval.y + z * rval.z;
    double mod1 = modulo();
    double mod2 = rval.modulo();

    /* a·b = |a|·|b| · cosθ */
    return std::acos(dotproduct / (mod1 * mod2));
}


Vector3D Vector3D::operator+(const Vector3D& rval) const
{
    return Vector3D(x + rval.x, y + rval.y, z + rval.z);
}

Vector3D Vector3D::operator-(const Vector3D& rval) const
{
    return Vector3D(x - rval.x, y - rval.y, z - rval.z);
}

Vector3D Vector3D::operator*(const Vector3D& rval) const
{
    /* Vector cross product: */
    return Vector3D(y * rval.z - z * rval.y, z * rval.x - x * rval.z, x * rval.y - y * rval.x);
}

Vector3D Vector3D::operator*(const double& k) const
{
    /* Multiplication by a constant: */
    return Vector3D(x * k, y * k, z * k);
}

Vector3D Vector3D::operator/(const double& k) const
{
    /* Divide by a constant: */
    return Vector3D(x / k, y / k, z / k);
}

Vector3D& Vector3D::operator+=(const Vector3D& rval)
{
    Vector3D v = *this + rval;
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

Vector3D& Vector3D::operator-=(const Vector3D& rval)
{
    Vector3D v = *this - rval;
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

Vector3D& Vector3D::operator*=(const Vector3D& rval)
{
    Vector3D v = *this * rval;
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

Vector3D& Vector3D::operator*=(const double& k)
{
    Vector3D v = *this * k;
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

Vector3D& Vector3D::operator/=(const double& k)
{
    Vector3D v = *this / k;
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}


bool Vector3D::operator==(const Vector3D& rval) const
{
    return x == rval.x && y == rval.y && z == rval.z;
}

bool Vector3D::operator!=(const Vector3D& rval) const
{
    return !(*this == rval);
}

bool Vector3D::operator<=(const Vector3D& rval) const
{
    return modulo() <= rval.modulo();
}

bool Vector3D::operator>=(const Vector3D& rval) const
{
    return modulo() >= rval.modulo();
}

bool Vector3D::operator<(const Vector3D& rval) const
{
    return !(*this >= rval);
}

bool Vector3D::operator>(const Vector3D& rval) const
{
    return !(*this <= rval);
}
