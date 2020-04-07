/***********************************************************************************************//**
 *  Miscelaneous math utilities.
 *  @class      MathUtils
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *              Joan Adrià Ruiz de Azúa (JARA), <joan.adria@tsc.upc.edu>
 *  @note       Ported from dss-sim and adapted.
 *  @date       2018-jan-29
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef MATH_UTILS_HPP
#define MATH_UTILS_HPP

#include "prot.hpp"

/***********************************************************************************************//**
 * It provides useful mathematical functions. It is able to perform the following transformations:
 *  - from degrees to radians, and viceversa
 *  - from arc-minutes to degrees, and viceversa
 *  - from arc-seconds to degrees, and viceversa
 *  - from arc-minutes to radians, and viceversa
 *  - from arc-seconds to radians, and viceversa
 *  - from seconds of time to radians, and viceversa
 *  - from revolution to arc-seconds
 *  - from degrees to seconds of time
 **************************************************************************************************/
class MathUtils
{
public:
    /*******************************************************************************************//**
     *  Transformation from degrees to radians.
     *  @param a     degrees to be converted [º]
     *  @return      radians resulting
     **********************************************************************************************/
    static long double degToRad(long double a);

    /*******************************************************************************************//**
     *  Transformation from radians to degrees.
     *  @param a     radians to be converted [radians]
     *  @return      degrees resulting [º]
     **********************************************************************************************/
    static long double radToDeg(long double a);

    /*******************************************************************************************//**
     *  Transformation from arc-minutes to degrees.
     *  @param a     arc-minutes to be converted [minutes of arc]
     *  @return      degrees resulting [º]
     **********************************************************************************************/
    static long double arcminToDeg(long double a);

    /*******************************************************************************************//**
     *  Transformation from arc-seconds to degrees.
     *  @param a     arc-seconds to be converted [seconds of arc]
     *  @return      degrees resulting [º]
     **********************************************************************************************/
    static long double arcsecToDeg(long double a);

    /*******************************************************************************************//**
     *  Transformation from degrees to arc-minutes.
     *  @param a     degrees to be converted [º]
     *  @return      arc-minutes resulting [minutes of arc]
     **********************************************************************************************/
    static long double degToArcmin(long double a);

    /*******************************************************************************************//**
     *  Transformation from degrees to arc-seconds.
     *  @param a     degrees to be converted [º]
     *  @return      arc-seconds resulting [seconds of arc]
     **********************************************************************************************/
    static long double degToArcsec(long double a);

    /*******************************************************************************************//**
     *  Transformation from arc-minutes to radians.
     *  @param a     arc-minutes to be converted [minutes of arc]
     *  @return      radians resulting
     **********************************************************************************************/
    static long double arcminToRad(long double a);

    /*******************************************************************************************//**
     *  Transformation from arc-seconds to radians.
     *  @param a     arc-second to be converted [seconds of arc]
     *  @return      radians resulting
     **********************************************************************************************/
    static long double arcsecToRad(long double a);

    /*******************************************************************************************//**
     *  Transformation from radians to arc-seconds.
     *  @param a     radians to be converted [radians]
     *  @return      arc-seconds resulting [seconds of arc]
     **********************************************************************************************/
    static long double radToArcsec(long double a);

    /*******************************************************************************************//**
     *  Transformation from revolution to arc-seconds.
     *  @param r     revolution to be converted
     *  @return      arc-seconds resulting [seconds of arc]
     **********************************************************************************************/
    static long double revToArcsec(long double r);

    /*******************************************************************************************//**
     *  Transformation from seconds of time to radians.
     *  @param a     seconds of time to be converted [seconds]
     *  @return      radians resulting
     **********************************************************************************************/
    static long double secTimeToRad(long double s);

    /*******************************************************************************************//**
     *  Transformation from radians to seconds of time.
     *  @param r     radians to be converted [radians]
     *  @return      seconds of time resulting [seconds]
     **********************************************************************************************/
    static long double radToSecTime(long double r);

    /*******************************************************************************************//**
     *  Transformation from degree to seconds of time.
     *  @param r     degrees to be converted [º]
     *  @return      seconds of time resulting [seconds]
     **********************************************************************************************/
    static long double degToSecTime(long double d);

    /*******************************************************************************************//**
     *  Computes the magnitude of a 2D vector (SFML).
     *  @param      v       Input 2D vector.
     **********************************************************************************************/
    template <typename T>
    static T norm(const sf::Vector2<T>& v);

    /*******************************************************************************************//**
     *  Computes the magnitude of a 3D vector (SFML).
     *  @param      v       Input 3D vector.
     **********************************************************************************************/
    template <typename T>
    static T norm(const sf::Vector3<T>& v);

    /*******************************************************************************************//**
     *  Computes the unitary vector in the direction of `v`.
     *  @tparam     Vec     An SFML vector (either sf::Vector2<T> or sf::Vector3<T>).
     *  @param      v       Vector to get direction from.
     **********************************************************************************************/
    template <class Vec>
    static Vec makeUnitary(const Vec& v);

    /*******************************************************************************************//**
     *  Computes cross-product (A x B) of two 3d vectors a and b.
     *  @tparam     T       The underlying type with which the vector components are expressed.
     *  @param      a       Vector A.
     *  @param      b       Vector B.
     **********************************************************************************************/
    template <class T>
    static sf::Vector3<T> cross(const sf::Vector3<T>& a, const sf::Vector3<T>& b);

    /*******************************************************************************************//**
     *  Computes dot-product (A · B) of two 3d vectors a and b.
     *  @tparam     T       The underlying type with which the vector components are expressed.
     *  @param      a       Vector A.
     *  @param      b       Vector B.
     **********************************************************************************************/
    template <class T>
    static T dot(const sf::Vector3<T>& a, const sf::Vector3<T>& b);

    /*******************************************************************************************//**
     *  Computes the great-circle arc between two unitary vectors a and b.
     *  @tparam     T       The underlying type with which the vector components are expressed.
     *  @param      a       Unitary vector A
     *  @param      b       Unitary vector B
     *  @return     The smaller great-circle arc between A and B, in radians.
     **********************************************************************************************/
    template <class T>
    static T arc(const sf::Vector3<T>& a, const sf::Vector3<T>& b);
};

template <typename T>
T MathUtils::norm(const sf::Vector2<T>& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

template <typename T>
T MathUtils::norm(const sf::Vector3<T>& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

template <class Vec>
Vec MathUtils::makeUnitary(const Vec& v)
{
    return v / norm(v);
}

template <class T>
sf::Vector3<T> MathUtils::cross(const sf::Vector3<T>& a, const sf::Vector3<T>& b)
{
    return sf::Vector3<T>(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

template <class T>
T MathUtils::dot(const sf::Vector3<T>& a, const sf::Vector3<T>& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <class T>
T MathUtils::arc(const sf::Vector3<T>& a, const sf::Vector3<T>& b)
{
    /* See: https://en.wikipedia.org/wiki/Great-circle_distance#Vector_version. */
    return std::atan2(norm(cross(a, b)), dot(a, b));
}

#endif  /* MATH_UTILS_HPP */
