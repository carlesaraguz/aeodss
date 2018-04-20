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

GASReward::GASReward(float t, float v)
    : time(t)
    , value(v)
{ }

GASReward GASReward::operator+(const GASReward& rval)
{
    GASReward retval = *this;
    retval.value += rval.value;
    return retval;
}

GASReward& GASReward::operator+=(const GASReward& rval)
{
    value += rval.value;
    return *this;
}

bool GASReward::operator>(const GASReward& rhs) const
{
    return time > rhs.time;
}

bool GASReward::operator<(const GASReward& rhs) const
{
    return time < rhs.time;
}

bool GASReward::operator>=(const GASReward& rhs) const
{
    return time  >=rhs.time;
}

bool GASReward::operator<=(const GASReward& rhs) const
{
    return time  <=rhs.time;
}

bool GASReward::operator==(const GASReward& rhs) const
{
    return time  ==rhs.time;
}

bool GASReward::operator!=(const GASReward& rhs) const
{
    return time  !=rhs.time;
}
