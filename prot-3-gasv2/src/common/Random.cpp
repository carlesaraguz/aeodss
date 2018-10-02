/***********************************************************************************************//**
 *  Random number generation utility.
 *  @class      Random
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-22
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Random.hpp"

std::random_device Random::m_rd;
std::mt19937 Random::m_uniform_gen;
std::uniform_real_distribution<float> Random::m_uniform_dist;

void Random::doInit(void)
{
    m_uniform_gen.seed(m_rd());
    m_uniform_dist = std::uniform_real_distribution<float>(0.f, 1.f);
}

float Random::getUf(void)
{
    return m_uniform_dist(m_uniform_gen);
}

float Random::getUf(float a, float b)
{
    if(a >= b) {
        return (a - b) * m_uniform_dist(m_uniform_gen) + b;
    } else {
        return (b - a) * m_uniform_dist(m_uniform_gen) + a;
    }
}

int Random::getUi(void)
{
    return m_uniform_dist(m_uniform_gen) * 100;
}

int Random::getUi(int a, int b)
{
    if(a >= b) {
        return (a - b) * m_uniform_dist(m_uniform_gen) + b;
    } else {
        return (b - a) * m_uniform_dist(m_uniform_gen) + a;
    }
}
