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

#include "MathUtils.hpp"

double MathUtils::degToRad(double a)
{
    return a * Config::pi / 180.0;
}

double MathUtils::radToDeg(double a)
{
    return a * 180.0 / Config::pi;
}

double MathUtils::arcminToDeg(double a)
{
    return a / 60.0;
}

double MathUtils::arcsecToDeg(double a)
{
    return a / 3600.0;
}

double MathUtils::degToArcmin(double a)
{
    return a * 60.0;
}

double MathUtils::degToArcsec(double a)
{
    return a * 3600.0;
}

double MathUtils::arcminToRad(double a)
{
    return degToRad(arcminToDeg(a));
}

double MathUtils::arcsecToRad(double a)
{
    return degToRad(arcsecToDeg(a));
}

double MathUtils::radToArcsec(double a)
{
    return degToArcsec(radToDeg(a));
}

double MathUtils::revToArcsec(double r)
{
    return 1296000 * r;
}

double MathUtils::secTimeToRad(double s)
{
    return s * Config::pi / 43200;
}

double MathUtils::radToSecTime(double r)
{
    return r * 43200 / Config::pi;
}

double MathUtils::degToSecTime(double d)
{
    return degToRad(radToSecTime(d));
}
