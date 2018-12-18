/***********************************************************************************************//**
 *  Utilities to handle and transform coordinates from on frame or reference to antoher.
 *  @class      CoordinateSystemUtils
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *              Joan Adrià Ruiz de Azúa (JARA), <joan.adria@tsc.upc.edu>
 *  @note       Ported from dss-sim and adapted to support sf::Vector3f.
 *  @date       2018-jan-26
 *  @version    0.2
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef COORDINATE_SYSTEM_UTILS_HPP
#define COORDINATE_SYSTEM_UTILS_HPP

#include "prot.hpp"

/* External includes */
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>

/* Internal includes */
#include "MathUtils.hpp"
#include "CoordinateSystemUtilsCoeff.hpp"
#include "VirtualTime.hpp"

/***********************************************************************************************//**
 * Provides frame transformation mechanisms. It is able to transform coordinates between
 * different frames:
 *  - [Earth-Centered Inertial (ECI) frame](https://en.wikipedia.org/wiki/Earth-centered_inertial)
 *  - [Earth-Centered Earth-Fixed (ECEF) frame](https://en.wikipedia.org/wiki/ECEF)
 *  - [Orbital frame](https://en.wikipedia.org/wiki/Orbital_elements)
 *  - [Geographic or Geodetic frame](https://en.wikipedia.org/wiki/Geographic_coordinate_system)
 * Note that the implemented transformations are the following ones:
 *  - from ECI to ECEF and viceversa
 *  - from Geographic to ECEF and viceversa
 *  - from ECI to Geographic and viceversa
 *  - from Orbital to ECI, or to ECEF, or to Geographic
 **************************************************************************************************/
class CoordinateSystemUtils
{
public:
    /*******************************************************************************************//**
     *  Transformation from ECI to ECEF. This conversion is based upon "Appendix - Transformation
     *  of ECI (CIS, EPOCH J2000.0) coordinates to WGS84 (CTS, ECEF) coordinates", from the
     *  National Geospatial-Intelligence Agency, available
     *  [on-line](http://earth-info.nga.mil/GandG/publications/tr8350.2/tr8350.2-a/Appendix.pdf).
     *  @param coord     Coordinates in ECI frame to be transformed
     *  @param jd        Current Julian Days of the transformation [days]
     **********************************************************************************************/
    static sf::Vector3f fromECIToECEF(sf::Vector3f coord, double jd);

    /*******************************************************************************************//**
     *  Transformation from ECEF to ECI. This conversion is based upon "Appendix - Transformation
     *  of ECI (CIS, EPOCH J2000.0) coordinates to WGS84 (CTS, ECEF) coordinates", from the
     *  National Geospatial-Intelligence Agency, available
     *  [on-line](http://earth-info.nga.mil/GandG/publications/tr8350.2/tr8350.2-a/Appendix.pdf).
     *  @param  coord   Coordinates in ECEF frame to be transformed
     *  @param  jd      Current Julian Days of the transformation [days]
     **********************************************************************************************/
    static sf::Vector3f fromECEFToECI(sf::Vector3f coord, double jd);

    /*******************************************************************************************//**
     *  Transformation from ECI to Geographic. This conversion uses an intermediate transformation
     *  from ECI to ECEF, and then from ECEF to Geographic.
     *  @param  coord   Coordinates in ECI frame to be transformed
     *  @param  jd      Current Julian Days of the transformation [days]
     *  @return         Geographic coordinates (x == lat, y == lng, z == h)
     **********************************************************************************************/
    static sf::Vector3f fromECIToGeographic(sf::Vector3f coord, double jd);

    /*******************************************************************************************//**
     *  Transformation from ECEF to Geographic. This conversion is based on the [Ferrari's solution
     *  using the Zhu formulation](https://en.wikipedia.org/wiki/Geographic_coordinate_conversion)
     *  @param coord    Coordinates in ECEF frame to be transformed
     *  @return         Geographic coordinates (x == lat, y == lng, z == h)
     **********************************************************************************************/
    static sf::Vector3f fromECEFToGeographic(sf::Vector3f coord);

    /*******************************************************************************************//**
     *  Transformation from Geographic to ECI. This conversion uses an intermediate transformation
     *  from Geographic to ECEF, and then from ECEF to ECI.
     *  @param coord        Coordinates in Geographic frame to be transformed (x == lat, y == lng,
     *                      z == height)
     *  @param jd           Current Julian Days of the transformation [days]
     **********************************************************************************************/
    static sf::Vector3f fromGeographicToECI(sf::Vector3f coord, double jd);

    /*******************************************************************************************//**
     *  Transformation from Geographic to ECEF. This conversion is based upon the latitude and
     *  longitude definitions, and it uses the [Earth ellipsoid World Geodetic System (WGS84) model]
     *  (https://en.wikipedia.org/wiki/World_Geodetic_System#A_new_World_Geodetic_System:_WGS_84).
     *  @param coord        Coordinates in Geographic frame to be transformed (x == lat, y == lng,
     *                      z == height)
     **********************************************************************************************/
    static sf::Vector3f fromGeographicToECEF(sf::Vector3f coord);

    /*******************************************************************************************//**
     *  Transformation from Orbital to ECI. This conversion is based upon the kepler orbital
     *  elements. It does not consider effects of the [flattering effect (J2-term)](
     *  https://en.wikipedia.org/wiki/Orbital_perturbation_analysis#The_effect_of_the_Earth_flattening)
     *  the argument of periapsis either right ascension of the ascending node.
     *  @param radius        Orbital coordinate to be transformed: radius.
     *  @param true_anomaly  Orbital coordinate to be transformed: true anomaly (in radians).
     *  @param right_asc     Right ascension of the ascending node of the orbit [º]
     *  @param arg_perigee   Argument of periapsis of the orbit [º]
     *  @param inclination   Inclination of the orbit [º]
     **********************************************************************************************/
    static sf::Vector3f fromOrbitalToECI(
        double radius,
        double true_anomaly,
        double right_asc,
        double arg_perigee,
        double inclination
    );

    /*******************************************************************************************//**
     *  Transformation from Orbital to ECEF. This conversion uses an intermediate transformation
     *  from Orbital to ECI, and then from ECI to ECEF.
     *  @param radius        Orbital coordinate to be transformed: radius.
     *  @param true_anomaly  Orbital coordinate to be transformed: true anomaly (in radians).
     *  @param jd            Current Julian Days of the transformation [days]
     *  @param right_asc     Right ascension of the ascending node of the orbit [º]
     *  @param arg_perigee   Argument of periapsis of the orbit [º]
     *  @param inclination   Inclination of the orbit [º]
     **********************************************************************************************/
    static sf::Vector3f fromOrbitalToECEF(
        double radius,
        double true_anomaly,
        double jd,
        double right_asc,
        double arg_perigee,
        double inclination
    );

    /*******************************************************************************************//**
     *  Transformation from Orbital to Geographic. This conversion uses intermediate transformations
     *  from Orbital to ECI, then from ECI to ECEF, and finally from ECEF to Geographic.
     *  @param radius        Orbital coordinate to be transformed: radius.
     *  @param true_anomaly  Orbital coordinate to be transformed: true anomaly (in radians).
     *  @param jd            Current Julian Days of the transformation [days]
     *  @param right_asc     Right ascension of the ascending node of the orbit [º]
     *  @param arg_perigee   Argument of periapsis of the orbit [º]
     *  @param inclination   Inclination of the orbit [º]
     **********************************************************************************************/
    static sf::Vector3f fromOrbitalToGeographic(
        double radius,
        double true_anomaly,
        double jd,
        double right_asc,
        double arg_perigee,
        double inclination
    );

private:
    /*******************************************************************************************//**
     *  Computes the Julian Centuries from the Julian Days. This function is used in the ECEF/ECI
     *  conversion process.
     *  @param jd            Current Julian Days of the transformation [days]
     **********************************************************************************************/
    static double getJulianCenturies(double jd);

    /*******************************************************************************************//**
     *  Computes the Greenwich Mean Sidereal Time (GSMT) from the Julian Days. This function is used
     *  in the ECEF/ECI conversion process.
     *  @param jd            Current Julian Days of the transformation [days]
     **********************************************************************************************/
    static double getGMST(double jd);

    /*******************************************************************************************//**
     *  Computes the precession matrix of the Earth. The Earth rotational axis changes through time
     *  and this matrix enables to characterize this perturbation. This function is used in the
     *  ECEF/ECI conversion process.
     *  @param jd            Current Julian Days of the transformation [days]
     **********************************************************************************************/
    static gsl_matrix* getPrecessionMatrix(double jd);

    /*******************************************************************************************//**
     *  Computes the nutation matrix of the Earth. The Earth rotational axis has a nodding motion
     *  which needs to be characterized during the ECEF/ECI conversion process.
     *  @param jd            Current Julian Days of the transformation [days]
     *  @param eps_ptr       Reference parameter to the true obliquity of ecliptic [radians]
     *  @param d_psi_ptr     Reference parameter to the nutation in longitude [radians]
     **********************************************************************************************/
    static gsl_matrix* getNutationMatrix(double jd, double* eps_ptr, double* d_psi_ptr);

    /*******************************************************************************************//**
     *  Computes the sideral matrix of the Earth. This function is used in the ECEF/ECI conversion
     *  process. Type of error from the gsl library
     *  @param jd        Current Julian Days of the transformation [days]
     *  @param eps       True obliquity of ecliptic [radians]
     *  @param d_psi     Nutation in longitude [radians]
     **********************************************************************************************/
    static gsl_matrix* getSideralMatrix(double jd, double eps, double d_psi);

    /*******************************************************************************************//**
     *  Computes a GSL matrix from a double pointer. This method reads the rows and the columns and
     *  constructs the GSL matrix accordingly to the input matrix. Note that the input i and j
     *  represent the number of rows and coulmns that have the input matrix. This method is used
     *  to work with other gsl functions.
     *  @param ptr   double pointer to the data matrix to be parsed
     *  @param i     number of matrix rows
     *  @param j     number of matrix columns
     **********************************************************************************************/
    static gsl_matrix* getGSLMatrixFromPtr(double * ptr, size_t i, size_t j);

    /*******************************************************************************************//**
     *  Computes the polar motion matrix of the Earth. As it does not exist a specific model
     *  to compute this matrix. Indeed, it is used a set of empiric measurements from IERS and JPL
     *  database. This function is used in the ECEF/ECI conversion process.
     *  @param jd        Current Julian Days of the transformation [days]
     **********************************************************************************************/
    static gsl_matrix* getPolarMotionMatrix(double jd);

    /*******************************************************************************************//**
     *  Internal handler to manage errors using GSL libraries.
     *  @param  reason      Error description.
     *  @param  file        File name in which has the error.
     *  @param  line        Line which has the error.
     *  @param  gsl_errno   Type of error from the gsl library.
     **********************************************************************************************/
    static void GSLErrorHandler(const char* reason, const char* file, int line, int gsl_errno);
};

#endif  /* COORDINATE_SYSTEM_UTILS_HPP */
