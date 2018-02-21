/***********************************************************************************************//**
 *  Static class with utilities.
 *  @class      Utils
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mmm-00
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef UTILS_HPP
#define UTILS_HPP

#include "prot.hpp"

#include "StringUtils.hpp"

class Utils
{
public:
    static double genRandom(double a, double b);
    static double genRandom(void);
    static std::string getRootPath(void);

private:
    /* For random number generation: */
    static std::random_device m_rand_dev;
    static std::default_random_engine m_rand_engine;
    static std::uniform_real_distribution<double> m_uniform_dist;


};

#endif /* UTILS_HPP */
