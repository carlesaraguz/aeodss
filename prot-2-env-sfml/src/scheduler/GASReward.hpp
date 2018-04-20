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
    GASReward(float t = 0.f, float v = 0.f);

    float time;
    float value;

    GASReward operator+(const GASReward &rval);
    GASReward& operator+=(const GASReward &rval);
    bool operator>(const GASReward& rhs) const;
    bool operator<(const GASReward& rhs) const;
    bool operator>=(const GASReward& rhs) const;
    bool operator<=(const GASReward& rhs) const;
    bool operator==(const GASReward& rhs) const;
    bool operator!=(const GASReward& rhs) const;
};

#endif /* GAS_REWARD_HPP */
