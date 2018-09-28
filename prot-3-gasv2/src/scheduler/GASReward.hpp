/***********************************************************************************************//**
 *  The reward type to compute GA fitnesses.
 *  @class      GASReward
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-16
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef GAS_REWARD_HPP
#define GAS_REWARD_HPP

#include "prot.hpp"

class GASReward
{
public:
    GASReward(void) = default;

    void setValue(unsigned int time_idx, float value);
    float consumeReward(unsigned int time_idx);
    void revert(void);

private:
    std::map<unsigned int, float> m_value;          /* Value of the reward at a given time-index. */
    std::map<unsigned int, float> m_value_backup;   /* Recover information. */
};

#endif /* GAS_REWARD_HPP */
