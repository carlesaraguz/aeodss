/***********************************************************************************************//**
 *  Interface for classes that are time-dependent.
 *  @class      TimeStep
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-06
 *  @version    0.2
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef TIME_STEP_HPP
#define TIME_STEP_HPP

#include "prot.hpp"

class TimeStep
{
public:
    virtual ~TimeStep(void) = default;
    virtual void step(void) = 0;
};

#endif /* TIME_STEP_HPP */
