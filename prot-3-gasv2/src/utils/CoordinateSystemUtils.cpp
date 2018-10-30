/***************************************************************************************************
*  File:        CoordinateSystemUtils.cpp                                                          *
*  Authors:     Carles Araguz (CA), <carles.araguz@upc.edu>                                        *
*               Joan Adrià Ruiz de Azúa (JARA). <joan.adria@tsc.upc.edu>                           *
*  Creation:    2018-jan-26                                                                        *
*  Description: Class that implements different mechanisms to parse between space coordinate       *
*               frames                                                                             *
*                                                                                                  *
*  This file is part of a project developed by Nano-Satellite and Payload Laboratory (NanoSat Lab) *
*  at Technical University of Catalonia - UPC BarcelonaTech.                                       *
* ------------------------------------------------------------------------------------------------ *
*  Changelog:                                                                                      *
*  v#   Date            Author  Description                                                        *
*  0.0  2017-may-23     CA      Implementation of ECI to ECEF methods in ECICoordonates.cpp        *
*  0.1  2018-jan-26     JARA    Creation.                                                          *
***************************************************************************************************/
#include "CoordinateSystemUtils.hpp"

sf::Vector3f CoordinateSystemUtils::fromECItoECEF(sf::Vector3f coord, double jd)
{
    /*  This conversion is based upon "Appendix - Transformation of ECI (CIS, EPOCH J2000.0)
     *  coordinates to WGS84 (CTS, ECEF) coordinates", from the National Geospatial-Intelligence
     *  Agency, available on-line at:
     *      http://earth-info.nga.mil/GandG/publications/tr8350.2/tr8350.2-a/Appendix.pdf
     **/
    double eps, d_psi;
    gsl_matrix* precession_mat = CoordinateSystemUtils::getPrecessionMatrix(jd);               /* D: 3x3 rotation matrix. */
    gsl_matrix* nutation_mat   = CoordinateSystemUtils::getNutationMatrix(jd, &eps, &d_psi);   /* C: 3x3 rotation matrix. */
    gsl_matrix* sideral_mat    = CoordinateSystemUtils::getSideralMatrix(jd, eps, d_psi);      /* B: 3x3 rotation matrix. */
    gsl_matrix* polar_mat      = CoordinateSystemUtils::getPolarMotionMatrix(jd);              /* A: 3x3 rotation matrix. */

    /* ECI to ECEF transformation:
     *  ECEF <-- A·B·C·D·ECI
     **/

    gsl_vector* ecef_vec = gsl_vector_alloc(3);
    gsl_vector* eci_vec  = gsl_vector_alloc(3);
    gsl_matrix* ab_mat   = gsl_matrix_alloc(3, 3);
    gsl_matrix* abc_mat  = gsl_matrix_alloc(3, 3);
    gsl_matrix* abcd_mat = gsl_matrix_alloc(3, 3);

    /* Compute ABCD */
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, polar_mat, sideral_mat, 0.0, ab_mat);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, ab_mat, nutation_mat, 0.0, abc_mat);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, abc_mat, precession_mat, 0.0, abcd_mat);

    gsl_vector_set_zero(ecef_vec);          /* Initialize ECEF vector.  */
    gsl_vector_set(eci_vec, 0, coord.x);    /* Initialize ECI vector.   */
    gsl_vector_set(eci_vec, 1, coord.y);    /* ...                      */
    gsl_vector_set(eci_vec, 2, coord.z);    /* ...                      */
    gsl_blas_dgemv(CblasNoTrans, 1.0, abcd_mat, eci_vec, 0.0, ecef_vec);     /* Computes ecef = (1 · abcd_mat · eci) */

    double x = gsl_vector_get(ecef_vec, 0);
    double y = gsl_vector_get(ecef_vec, 1);
    double z = gsl_vector_get(ecef_vec, 2);

    gsl_vector_free(ecef_vec);
    gsl_vector_free(eci_vec);
    gsl_matrix_free(precession_mat);
    gsl_matrix_free(nutation_mat);
    gsl_matrix_free(sideral_mat);
    gsl_matrix_free(ab_mat);
    gsl_matrix_free(abc_mat);
    gsl_matrix_free(abcd_mat);

    return sf::Vector3f(x, y, z);
}

sf::Vector3f CoordinateSystemUtils::fromECItoGeographic(sf::Vector3f coord, double jd)
{
    return fromECEFtoGeographic(fromECItoECEF(coord, jd));
}

sf::Vector3f CoordinateSystemUtils::fromECEFtoGeographic(sf::Vector3f coord)
{
    /* The conversion from ECEF to Geographic coordinates is performed using the Ferrari's
     * conversion. In particular, the following algorithm implement the five steps defined by
     * Kaplan (see [ref](https://en.wikipedia.org/wiki/Geographic_coordinate_conversion)).
     **/
    double a = Config::earth_wgs84_a;
    double e = Config::earth_wgs84_e;
    double b = Config::earth_wgs84_b;
    double r = std::sqrt(coord.x * coord.x + coord.y * coord.y);
    double e_ = std::sqrt((a * a - b * b) / (b * b));
    double E = std::sqrt(a * a - b * b);
    double F = 54 * std::pow(b * coord.z, 2);
    double G = std::pow(r, 2) + (1 - e * e) * std::pow(coord.z, 2) - std::pow(e * E, 2);
    double C = std::pow(e, 4) * F * std::pow(r, 2) / std::pow(G, 3);
    double S = std::pow((1 + C + std::sqrt(C * (C + 2))), 1.0 / 3);
    double P = F / (3 * std::pow(G * (S + 1 / S + 1), 2));
    double Q = std::sqrt(1 + 2 * P * std::pow(e, 4));
    double ro = -P * r * r / 2;
    ro -= P * (1 - e * e) * coord.z * coord.z / (Q * (1 + Q));
    ro += std::pow(a, 2) * (1 + 1 / Q) * 0.5;
    ro = -(P * e * e * r) / (1 + Q) + std::sqrt(ro);
    double U = std::sqrt(std::pow(r - e * e * ro, 2) + coord.z * coord.z);
    double V = std::sqrt(std::pow(r - e * e * ro, 2) + (1 - e * e) * coord.z * coord.z);
    double Zo = b * b * coord.z / (a * V);
    double h = U * (1 - (b * b / (a * V)));
    double lat = std::atan((coord.z + e_ * e_ * Zo) / r);
    double lon = std::atan2(coord.y, coord.x);

    return sf::Vector3f(MathUtils::radToDeg(lat), MathUtils::radToDeg(lon), h);
}

sf::Vector3f CoordinateSystemUtils::fromOrbitaltoECI(
    double radius,
    double true_anomaly,
    double right_asc,
    double arg_perigee,
    double inclination
)
{
    double raan = MathUtils::degToRad(right_asc);
    double argp = MathUtils::degToRad(arg_perigee);
    double incl = MathUtils::degToRad(inclination);
    double x, y, z;

    x  = radius * std::cos(true_anomaly) * ( std::cos(raan) * std::cos(argp) - std::sin(raan) * std::sin(argp) * std::cos(incl));
    x += radius * std::sin(true_anomaly) * (-std::cos(raan) * std::sin(argp) - std::sin(raan) * std::cos(argp) * std::cos(incl));

    y  = radius * std::cos(true_anomaly) * ( std::sin(raan) * std::cos(argp) + std::cos(raan) * std::sin(argp) * std::cos(incl));
    y += radius * std::sin(true_anomaly) * (-std::sin(raan) * std::sin(argp) + std::cos(raan) * std::cos(argp) * std::cos(incl));

    z  = radius * std::sin(true_anomaly) * std::sin(incl) * std::cos(argp);
    z += radius * std::cos(true_anomaly) * std::sin(incl) * std::sin(argp);

    return sf::Vector3f(x, y, z);
}

sf::Vector3f CoordinateSystemUtils::fromOrbitaltoECEF(
    double radius,
    double true_anomaly,
    double jd,
    double right_asc,
    double arg_perigee,
    double inclination
)
{
    return fromECItoECEF(fromOrbitaltoECI(radius, true_anomaly, right_asc, arg_perigee, inclination), jd);
}

sf::Vector3f CoordinateSystemUtils::fromOrbitaltoGeographic(
    double radius,
    double true_anomaly,
    double jd,
    double right_asc,
    double arg_perigee,
    double inclination
)
{
    return fromECItoGeographic(fromOrbitaltoECI(radius, true_anomaly, right_asc, arg_perigee, inclination), jd);
}


gsl_matrix * CoordinateSystemUtils::getPrecessionMatrix(double jd)
{
    double c  = CoordinateSystemUtils::getJulianCenturies(jd);     /* c is T: Julian centuries from epoch JD2000. */
    double c2 = c * c;                      /* T². */
    double c3 = c * c * c;                  /* T³. */

    /* Precession D = Rz[-(90+z)] · Rx[θ] · R[90-ξ] ------------------------------------------------
     *  ξ   : xi
     *  z   : zz
     *  θ   : th
     **/
    double xi = MathUtils::arcsecToRad(2306.2181 * c + 0.30188 * c2 + 0.017998 * c3); /* arc-seconds -> radians. */
    double zz = MathUtils::arcsecToRad(2306.2181 * c + 1.09468 * c2 + 0.018203 * c3); /* arc-seconds -> radians. */
    double th = MathUtils::arcsecToRad(2004.3109 * c - 0.42665 * c2 - 0.041833 * c3); /* arc-seconds -> radians. */
    double precession[] = {
        /* Row 0: */
         std::cos(zz) * std::cos(th) * std::cos(xi) - std::sin(zz) * std::sin(xi),
        -std::cos(zz) * std::cos(th) * std::sin(xi) - std::sin(zz) * std::cos(xi),
        -std::cos(zz) * std::sin(th),
        /* Row 1: */
         std::sin(zz) * std::cos(th) * std::cos(xi) + std::cos(zz) * std::sin(xi),
        -std::sin(zz) * std::cos(th) * std::sin(xi) + std::cos(zz) * std::cos(xi),
        -std::sin(zz) * std::sin(th),
        /* Row 2: */
         std::sin(th) * std::cos(xi),
        -std::sin(th) * std::sin(xi),
         std::cos(th)
    };
    return CoordinateSystemUtils::getGSLMatrixFromPtr(precession, 3, 3);
}

double CoordinateSystemUtils::getJulianCenturies(double jd)
{
    /*  JED(JD2000) = 2451545.0 = jd_2000
     *  Julian centuries = (JED - JD2000) / (days in 1 century).
     **/
    constexpr double jd2000 = 2451545.0;    /* Julian Date 2000. */
    constexpr double jcentury = 36525;      /* Number of days in a Julian century. */
    double jed;     /* Julian Ephemeris Date: the Julian date of the previous midnight. */
    if(jd < std::floor(jd) + 0.5) {
        jed = std::floor(jd) - 0.5;
    } else {
        jed = std::floor(jd) + 0.5;
    }
    double d = jed - jd2000;    /* Julian ephemeris days from epoch JD2000. */
    return d / jcentury;        /* Julian centuries from epoch JD2000, T. */
}

double CoordinateSystemUtils::getGMST(double jd)
{
    constexpr double jd2000 = 2451545.0;    /* Julian Date 2000.                        */
    constexpr double jcentury = 36525.0;    /* Number of days in a Julian century.      */
    double jed;     /* Julian Ephemeris Date: the Julian date of the previous midnight. */
    if(jd < std::floor(jd) + 0.5) {
        jed = std::floor(jd) - 0.5;
    } else {
        jed = std::floor(jd) + 0.5;
    }
    double du = jed - jd2000;               /* Julian ephemeris days from epoch JD2000. */
    double tu = du / jcentury;              /* Julian centuries from epoch JD2000, Tu.  */
    double tu2 = tu * tu;                   /* Tu². */
    double tu3 = tu * tu * tu;              /* Tu³. */


    return 24110.54841 + 8640184.812866 * tu + 0.093104 * tu2 - 6.2e-6 * tu3; /* seconds. */
}

gsl_matrix * CoordinateSystemUtils::getPolarMotionMatrix(double jd)
{
    /* Pair with the <X_p, Y_p> values */
    std::tuple<double, double> perturbations = CoordinateSystemUtilsCoeff::getPolarPerturbations(jd);
    double x_p = std::get<0>(perturbations);
    double y_p = std::get<1>(perturbations);

    /*  The computation of the matrix values is performed following the definition in [1].
     *  However, given that x_p and y_p are very small, approximated trigonometric functions can be
     *  applied (e.g., sin(x) ≈ x). This simplification, suggested in [2], yields the matrix in
     *  `motion`. Non-approximated rotation matrix implementations would be written as follows:
     *
     *  motion[] = {
     *       cos(y_p),              0,          -sin(y_p),
     *       sin(x_p) * sin(y_p),   cos(x_p),   cos(y_p) * sin(x_p),
     *       cos(x_p) * sin(y_p),   -sin(x_p),  cos(x_p) * cos(y_p)
     *  };
     *
     *  [1] http://www.navipedia.net/index.php/Transformation_between_Celestial_and_Terrestrial_Frames.
     *  [2] ["Appendix - Transformation of ECI (CIS, EPOCH J2000.0) coordinates to WGS84 (CTS, ECEF)
     *      coordinates"](http://earth-info.nga.mil/GandG/publications/tr8350.2/tr8350.2-a/Appendix.pdf)
     **/
    double motion[] = {
        1,    0,   x_p,     /* Row 0 */
        0,    1,   -y_p,    /* Row 1 */
        -x_p, y_p, 1        /* Row 2 */
    };
    return CoordinateSystemUtils::getGSLMatrixFromPtr(motion, 3, 3);
}

gsl_matrix * CoordinateSystemUtils::getNutationMatrix(double jd, double* eps_ptr = nullptr, double* d_psi_ptr = nullptr)
{
    double c  = CoordinateSystemUtils::getJulianCenturies(jd);     /* c is T: Julian centuries from epoch JD2000. */
    double c2 = c * c;                      /* T². */
    double c3 = c * c * c;                  /* T³. */

    /* Astronomic nutation C = Rx[-ε] · Rz[-Δψ] · Rx[m(ε)]
     *  m(ε): m_eps --> Mean obliquity of ecliptic.
     *  Δψ  : d_psi --> Nutation in longitude.
     *  Δε  : d_eps --> Nutation in obliquity.
     *  ε   : eps   --> True obliquity of ecliptic.
     *
     *  Arguments:
     *  ℓ   : mam --> Mean anomaly of Moon.
     *  ℓ'  : mas --> Mean anomaly of Sun.
     *  F   : mlm --> Mean longitude of Moon - Ω.
     *  D   : memfs --> Mean elongation of moon from Sun.
     *  Ω   : omega --> Longitude of ascending node of lunar mean orbit on ecliptic, measured from mean equinox of date.
     **/
    double m_eps = MathUtils::arcsecToRad(84381.448 - 46.8150 * c + 0.00059 * c2 + 0.001813 * c2); /* arc-seconds -> radians. */
    double d_eps = 0.0;
    double d_psi = 0.0;

    double mam   =  485866.733 + (MathUtils::revToArcsec(1325) +  715922.633) * c + 31.310 * c2 + 0.064 * c3; /* arc-seconds. */
    double mas   = 1287099.804 + (MathUtils::revToArcsec(  99) + 1292581.244) * c -  0.577 * c2 - 0.012 * c3; /* arc-seconds. */
    double mlm   =  335778.877 + (MathUtils::revToArcsec(1342) +  295263.137) * c - 13.257 * c2 + 0.011 * c3; /* arc-seconds. */
    double memfs = 1072261.307 + (MathUtils::revToArcsec(1236) + 1105601.328) * c -  6.891 * c2 + 0.019 * c3; /* arc-seconds. */
    double omega =  450160.280 - (MathUtils::revToArcsec(   5) +  482890.539) * c +  7.455 * c2 + 0.008 * c3; /* arc-seconds. */

    double coeff_a1i, coeff_a2i, coeff_a3i, coeff_a4i, coeff_a5i, coeff_Ai, coeff_Bi, coeff_Ci, coeff_Di, sin_arg;
    for(int i = 0; i < ECI_ECEF_COEFF_COUNT; i++) {
        /* Retrieve coefficients from look-up-tables: */
        coeff_a1i = CoordinateSystemUtilsCoeff::a1_tab[i];
        coeff_a2i = CoordinateSystemUtilsCoeff::a2_tab[i];
        coeff_a3i = CoordinateSystemUtilsCoeff::a3_tab[i];
        coeff_a4i = CoordinateSystemUtilsCoeff::a4_tab[i];
        coeff_a5i = CoordinateSystemUtilsCoeff::a5_tab[i];
        coeff_Ai  = CoordinateSystemUtilsCoeff::coeff_A_tab[i];
        coeff_Bi  = CoordinateSystemUtilsCoeff::coeff_B_tab[i];
        coeff_Ci  = CoordinateSystemUtilsCoeff::coeff_C_tab[i];
        coeff_Di  = CoordinateSystemUtilsCoeff::coeff_D_tab[i];

        sin_arg  = coeff_a1i * mam;
        sin_arg += coeff_a2i * mas;
        sin_arg += coeff_a3i * mlm;
        sin_arg += coeff_a4i * memfs;
        sin_arg += coeff_a5i * omega;

        /* Contribute to Δψ sum: */
        d_psi += (coeff_Ai + coeff_Bi * c) * std::sin(MathUtils::arcsecToRad(sin_arg));

        /* Contribute to Δε sum: */
        d_eps += (coeff_Ci + coeff_Di * c) * std::sin(MathUtils::arcsecToRad(sin_arg));
    }
    d_psi = MathUtils::arcsecToRad(0.0001 * d_psi);   /* 0.0001 arc-seconds -> radians. */
    d_eps = MathUtils::arcsecToRad(0.0001 * d_eps);   /* 0.0001 arc-seconds -> radians. */
    double eps = m_eps + d_eps;     /* radians + radians -> radians. */
    if(eps_ptr != nullptr) {
        *eps_ptr = eps;
    }
    if(d_psi_ptr != nullptr) {
        *d_psi_ptr = d_psi;
    }

    double nutation[] = {
        /* Row 0: */
        std::cos(d_psi),
        -std::sin(d_psi) * std::cos(m_eps),
        -std::sin(d_psi) * std::sin(m_eps),
        /* Row 1: */
        std::cos(eps) * std::sin(d_psi),
        std::cos(eps) * std::cos(d_psi) * std::cos(m_eps) + std::sin(eps) * std::sin(m_eps),
        std::cos(eps) * std::cos(d_psi) * std::sin(m_eps) - std::sin(eps) * std::cos(m_eps),
        /* Row 2: */
        std::sin(eps) * std::sin(d_psi),
        std::sin(eps) * std::cos(d_psi) * std::cos(m_eps) - std::cos(eps) * std::sin(m_eps),
        std::sin(eps) * std::cos(d_psi) * std::sin(m_eps) + std::cos(eps) * std::cos(m_eps)
    };

    return CoordinateSystemUtils::getGSLMatrixFromPtr(nutation, 3, 3);
}

gsl_matrix * CoordinateSystemUtils::getSideralMatrix(double jd, double eps = 0.0, double d_psi = 0.0)
{
    double c  = CoordinateSystemUtils::getJulianCenturies(jd);     /* c is T: Julian centuries from epoch JD2000. */

    /* Sideral time B = Rz[Λ]
     *  Λ   : lambda --> Longitude of the Zero Meridian from the true vernal equinox of date.
     *  H0  : gst --> Greenwich mean sideral time at 0h UT1 of JED.
     *  ΔH  : d_h --> (Apparent minus mean) sideral time.
     *  ω*  : w_rrprf --> Rotation rate in precessing reference frame.
     *  ω'  : w_eirr  --> Earth's intertial rotation rate.
     *  m   : m    --> Rate of precession in right ascension.
     *  t   : twd  --> Time within day (in UTC).
     *  Δt  : dut1 --> Difference between UTC and UT1. Considered constant (!)
     **/
    double int_part;
    double dut1 = -CURRENT_DUT1;                                /* UTC-UT1 = -DUT1 (seconds.)   */
    double twd  = std::modf((jd - 0.5), &int_part) * 24 * 3600.0;    /* Time within day, in seconds. */
    double gst  = MathUtils::secTimeToRad(getGMST(jd));              /* radians. */
    double d_h  = std::atan(std::cos(eps) * std::tan(d_psi));        /* radians. */

    constexpr double w_eirr = 7.2921151467e-5;  /* radians/second. */
    double m = 7.086e-12 + 4.3e-15 * c;         /* radians/second. */
    double w_rrprf = w_eirr + m;                /* radians/second. */

    double lambda = gst + d_h + w_rrprf * (twd - dut1);

    double sideral[] = {
         std::cos(lambda), std::sin(lambda), 0, /* Row 0: */
        -std::sin(lambda), std::cos(lambda), 0, /* Row 1: */
                        0,                0, 1  /* Row 2: */
    };

    return CoordinateSystemUtils::getGSLMatrixFromPtr(sideral, 3, 3);
}

gsl_matrix * CoordinateSystemUtils::getGSLMatrixFromPtr(double * ptr, size_t i, size_t j)
{
    gsl_matrix * ret = gsl_matrix_calloc(i, j);
    for(std::size_t ii = 0; ii < i; ii++) {
        for(std::size_t jj = 0; jj < j; jj++) {
            gsl_matrix_set(ret, ii, jj, ptr[i * ii + jj]);
        }
    }
    return ret;
}
