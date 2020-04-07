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

long double MathUtils::degToRad(long double a)
{
    return a * Config::pi / 180.0;
}

long double MathUtils::radToDeg(long double a)
{
    return a * 180.0 / Config::pi;
}

long double MathUtils::arcminToDeg(long double a)
{
    return a / 60.0;
}

long double MathUtils::arcsecToDeg(long double a)
{
    return a / 3600.0;
}

long double MathUtils::degToArcmin(long double a)
{
    return a * 60.0;
}

long double MathUtils::degToArcsec(long double a)
{
    return a * 3600.0;
}

long double MathUtils::arcminToRad(long double a)
{
    return degToRad(arcminToDeg(a));
}

long double MathUtils::arcsecToRad(long double a)
{
    return degToRad(arcsecToDeg(a));
}

long double MathUtils::radToArcsec(long double a)
{
    return degToArcsec(radToDeg(a));
}

long double MathUtils::revToArcsec(long double r)
{
    return 1296000 * r;
}

long double MathUtils::secTimeToRad(long double s)
{
    return s * Config::pi / 43200;
}

long double MathUtils::radToSecTime(long double r)
{
    return r * 43200 / Config::pi;
}

long double MathUtils::degToSecTime(long double d)
{
    return degToRad(radToSecTime(d));
}
