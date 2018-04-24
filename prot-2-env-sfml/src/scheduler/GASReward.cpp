/***********************************************************************************************//**
 *  The reward type to compute GA fitnesses.
 *  @class      GASReward
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-16
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "GASReward.hpp"

GASReward::GASReward(int tidx)
    : time_idx(tidx)
{ }

float GASReward::getReward(std::map<unsigned int, bool>& cells) const
{
    float acc = 0.f;
    for(auto& v : m_values) {
        acc = std::max(cells[v.first] * v.second, acc);
        cells[v.first] = false;     /* Disable this reward. */
    }
    return acc;
}
