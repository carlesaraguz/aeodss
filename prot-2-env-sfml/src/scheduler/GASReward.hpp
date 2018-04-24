/***********************************************************************************************//**
 *  The reward type to compute GA fitnesses.
 *  @class      GASReward
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-16
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef GAS_REWARD_HPP
#define GAS_REWARD_HPP

#include "prot.hpp"

class GASReward
{
public:
    int time_idx;

    GASReward(int tidx = 0);

    void setValues(std::map<unsigned int, float> vals) { m_values = vals; }
    float getReward(std::map<unsigned int, bool>& cells) const;

private:
    std::map<unsigned int, float> m_values;
};

#endif /* GAS_REWARD_HPP */
