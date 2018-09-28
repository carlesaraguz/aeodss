/***********************************************************************************************//**
 *  Random number generation utility.
 *  @class      Random
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-22
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef RANDOM_HPP
#define RANDOM_HPP

#include "prot.hpp"

class Random
{
public:
    static float getUf(void);
    static float getUf(float a, float b);
    static int getUi(void);
    static int getUi(int a, int b);
    static void doInit(void);
private:
    static std::random_device m_rd;
    static std::mt19937 m_uniform_gen;
    static std::uniform_real_distribution<float> m_uniform_dist;
};

#endif /* RANDOM_HPP */
