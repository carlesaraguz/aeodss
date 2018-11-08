/***********************************************************************************************//**
 *  Coefficients for ECI<->ECEF transformations in CoordinateSystemUtils.
 *  @class      CoordinateSystemUtilsCoeff
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *              Joan Adrià Ruiz de Azúa (JARA), <joan.adria@tsc.upc.edu>
 *  @note       Ported from dss-sim and adapted to support sf::Vector3f.
 *  @date       2018-may-23
 *  @version    0.3
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef COORDINATE_SYSTEM_UTILS_COEFF_HPP
#define COORDINATE_SYSTEM_UTILS_COEFF_HPP

#include "prot.hpp"
#include "MathUtils.hpp"
#include <tuple>

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
