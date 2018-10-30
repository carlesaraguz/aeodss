/***************************************************************************************************
*  File:        MathUtils.hpp                                                                      *
*  Authors:     Joan Adrià Ruiz de Azúa (JARA). <joan.adria@tsc.upc.edu>                           *
*  Creation:    2018-jan-29                                                                        *
*  Description: Class that implements different mathematical operations which are useful           *
*                                                                                                  *
*  This file is part of a project developed by Nano-Satellite and Payload Laboratory (NanoSat Lab) *
*  at Technical University of Catalonia - UPC BarcelonaTech.                                       *
* ------------------------------------------------------------------------------------------------ *
*  Changelog:                                                                                      *
*  v#   Date            Author  Description                                                        *
*  0.1  2018-jan-29     JARA    Creation.                                                          *
***************************************************************************************************/
#ifndef __MATH_UTILS_HPP__
#define __MATH_UTILS_HPP__

/* Internal includes */
#include "Config.hpp"

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
     * Transformation from degrees to radians.
     *
     * @param a     degrees to be converted [º]
     * @return      radians resulting
     **********************************************************************************************/
    static double degToRad(double a);

    /*******************************************************************************************//**
     * Transformation from radians to degrees.
     *
     * @param a     radians to be converted [radians]
     * @return      degrees resulting [º]
     **********************************************************************************************/
    static double radToDeg(double a);

    /*******************************************************************************************//**
     * Transformation from arc-minutes to degrees.
     *
     * @param a     arc-minutes to be converted [minutes of arc]
     * @return      degrees resulting [º]
     **********************************************************************************************/
    static double arcminToDeg(double a);

    /*******************************************************************************************//**
     * Transformation from arc-seconds to degrees.
     *
     * @param a     arc-seconds to be converted [seconds of arc]
     * @return      degrees resulting [º]
     **********************************************************************************************/
    static double arcsecToDeg(double a);

    /*******************************************************************************************//**
     * Transformation from degrees to arc-minutes.
     *
     * @param a     degrees to be converted [º]
     * @return      arc-minutes resulting [minutes of arc]
     **********************************************************************************************/
    static double degToArcmin(double a);

    /*******************************************************************************************//**
     * Transformation from degrees to arc-seconds.
     *
     * @param a     degrees to be converted [º]
     * @return      arc-seconds resulting [seconds of arc]
     **********************************************************************************************/
    static double degToArcsec(double a);

    /*******************************************************************************************//**
     * Transformation from arc-minutes to radians.
     *
     * @param a     arc-minutes to be converted [minutes of arc]
     * @return      radians resulting
     **********************************************************************************************/
    static double arcminToRad(double a);

    /*******************************************************************************************//**
     * Transformation from arc-seconds to radians.
     *
     * @param a     arc-second to be converted [seconds of arc]
     * @return      radians resulting
     **********************************************************************************************/
    static double arcsecToRad(double a);

    /*******************************************************************************************//**
     * Transformation from radians to arc-seconds.
     *
     * @param a     radians to be converted [radians]
     * @return      arc-seconds resulting [seconds of arc]
     **********************************************************************************************/
    static double radToArcsec(double a);

    /*******************************************************************************************//**
     * Transformation from revolution to arc-seconds.
     *
     * @param r     revolution to be converted
     * @return      arc-seconds resulting [seconds of arc]
     **********************************************************************************************/
    static double revToArcsec(double r);

    /*******************************************************************************************//**
     * Transformation from seconds of time to radians.
     *
     * @param a     seconds of time to be converted [seconds]
     * @return      radians resulting
     **********************************************************************************************/
    static double secTimeToRad(double s);

    /*******************************************************************************************//**
     * Transformation from radians to seconds of time.
     *
     * @param r     radians to be converted [radians]
     * @return      seconds of time resulting [seconds]
     **********************************************************************************************/
    static double radToSecTime(double r);

    /*******************************************************************************************//**
     * Transformation from degree to seconds of time.
     *
     * @param r     degrees to be converted [º]
     * @return      seconds of time resulting [seconds]
     **********************************************************************************************/
    static double degToSecTime(double d);
};

#endif  /* __MATH_UTILS_HPP__ */
