/***************************************************************************************************
*  File:        CoordinateSystemUtilsCoeff.hpp                                                     *
*  Authors:     Carles Araguz (CA), <carles.araguz@upc.edu>                                        *
*               Joan Adrià Ruiz de Azúa (JARA), <joan.adria@tsc.upc.edu>                           *
*  Creation:    2017-may-23                                                                        *
*  Description: Coefficients for the obtainment of rotation matrices to convert ECI<->ECEF.        *
*                                                                                                  *
*  This file is part of a project developed by Nano-Satellite and Payload Laboratory (NanoSat Lab) *
*  at Technical University of Catalonia - UPC BarcelonaTech.                                       *
* ------------------------------------------------------------------------------------------------ *
*  Changelog:                                                                                      *
*  v#   Date            Author      Description                                                    *
*  0.1  2017-may-23     CA          Creation.                                                      *
*  0.2  2018-jan-26     JARA        Changed name from ECICoordinatesCoeff                          *
*  0.3  2018-feb-12     JARA        Included polar motion perturbations                            *
***************************************************************************************************/

#ifndef __COORDINATE_SYSTEM_UTILS_COEFF_HPP__
#define __COORDINATE_SYSTEM_UTILS_COEFF_HPP__

#include "MathUtils.hpp"
#include <tuple>
#include <math.h>
#include "Config.hpp"

#define ECI_ECEF_COEFF_COUNT    106
#define IERS_DATABASE_COUNT     1460
#define CURRENT_DUT1    0.56    /* Current UT1-UTC value (in seconds.) */
/* DUT1 is taken from:
 *  https://www.nist.gov/pml/time-and-frequency-division/atomic-standards/leap-second-and-ut1-utc-information
 *  Accessed 2017-mar-27.
 **/

struct CoordinateSystemUtilsCoeff
{
public:
    static int a1_tab[ECI_ECEF_COEFF_COUNT];
    static int a2_tab[ECI_ECEF_COEFF_COUNT];
    static int a3_tab[ECI_ECEF_COEFF_COUNT];
    static int a4_tab[ECI_ECEF_COEFF_COUNT];
    static int a5_tab[ECI_ECEF_COEFF_COUNT];
    static double coeff_A_tab[ECI_ECEF_COEFF_COUNT];
    static double coeff_B_tab[ECI_ECEF_COEFF_COUNT];
    static double coeff_C_tab[ECI_ECEF_COEFF_COUNT];
    static double coeff_D_tab[ECI_ECEF_COEFF_COUNT];
    static std::tuple<double, double> getPolarPerturbations(double jd);

private:
    static double iers_database[IERS_DATABASE_COUNT];
};


#endif  /* __COORDINATE_SYSTEM_UTILS_COEFF_HPP__ */
