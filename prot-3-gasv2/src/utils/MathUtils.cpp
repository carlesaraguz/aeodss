#include "MathUtils.hpp"

double MathUtils::degToRad(double a)
{
    /* Replaces #define DEG_TO_RAD(a) ((a) * PI / 180.0) */
    return a * Config::pi / 180.0;
}

double MathUtils::radToDeg(double a)
{
    /* Replaces #define RAD_TO_DEG(a) ((a) * 180.0 / PI) */
    return a * 180.0 / Config::pi;
}

double MathUtils::arcminToDeg(double a)
{
    /* Replaces #define ARCMIN_TO_DEG(a) ((a) / 60.0) */
    return a / 60.0;
}

double MathUtils::arcsecToDeg(double a)
{
    /* Replaces #define ARCSEC_TO_DEG(a) ((a) / 3600.0) */
    return a / 3600.0;
}

double MathUtils::degToArcmin(double a)
{
    /* Replaces #define DEG_TO_ARCMIN(a) ((a) * 60.0) */
    return a * 60.0;
}

double MathUtils::degToArcsec(double a)
{
    /* Replaces #define DEG_TO_ARCSEC(a) ((a) * 3600.0) */
    return a * 3600.0;
}

double MathUtils::arcminToRad(double a)
{
    /* Replaces #define ARCMIN_TO_RAD(a) DEG_TO_RAD( ARCMIN_TO_DEG( (a) ) ) */
    return degToRad(arcminToDeg(a));
}

double MathUtils::arcsecToRad(double a)
{
    /* Replaces #define ARCSEC_TO_RAD(a) DEG_TO_RAD( ARCSEC_TO_DEG( (a) ) ) */
    return degToRad(arcsecToDeg(a));
}

double MathUtils::radToArcsec(double a)
{
    /* Replaces #define RAD_TO_ARCSEC(a) DEG_TO_ARCSEC( RAD_TO_DEG( (a) ) ) */
    return degToArcsec(radToDeg(a));
}

double MathUtils::revToArcsec(double r)
{
    /* Replaces #define REV_TO_ARCSEC(r) (1296000 * (r)) */
    return 1296000 * r;
}

double MathUtils::secTimeToRad(double s)
{
    /* Replaces #define SECT_TO_RAD(s) ((s) * PI / 43200) */
    return s * Config::pi / 43200;
}

double MathUtils::radToSecTime(double r)
{
    /* Replaces #define RAD_TO_SECT(r) ((r) * 43200 / PI) */
    return r * 43200 / Config::pi;
}

double MathUtils::degToSecTime(double d)
{
    /* Replaces #define DEG_TO_SECT(d) DEG_TO_RAD( RAD_TO_SECT( (d) ) ) */
    return degToRad(radToSecTime(d));
}
