/***********************************************************************************************//**
 *  Common enum class types.
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-23
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef COMMON_ENUM_TYPES_HPP
#define COMMON_ENUM_TYPES_HPP

#include "prot.hpp"

enum class Aggregate {
    MAX_VALUE,
    MIN_VALUE,
    MEAN_VALUE,
};

enum class AgentMotionType {
    LINEAR_BOUNCE,      /* lb */
    LINEAR_INFINITE,    /* li - unimplemented. */
    SINUSOIDAL,         /* sin - unimplemented. */
    ORBITAL             /* orb - unimplemented. */
};

#endif /* COMMON_ENUM_TYPES_HPP */
