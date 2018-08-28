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

void GASReward::setValue(unsigned int time_idx, float value)
{
    m_value[time_idx] = value;
    m_value_backup[time_idx] = value;
}

float GASReward::consumeReward(unsigned int time_idx)
{
    float retval = 0.f;
    try {
        /*  Access with std::map::at has logarithmic complexity but the size of this map is
        *  expected to be small.
        **/
        retval = m_value.at(time_idx);
    } catch(std::out_of_range& e) {
        std::cerr << "Genetic Algorithm Scheduler Error: un-registered reward value for time index " << time_idx << ".\n";
        retval = 0.f;
    }
    return retval;
}
