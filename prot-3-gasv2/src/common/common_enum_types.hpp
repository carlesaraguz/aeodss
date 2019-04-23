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
    SUM_VALUE,
};

enum class AgentMotionType {
    LINEAR_BOUNCE,      /* lb */
    LINEAR_INFINITE,    /* li - unimplemented. */
    SINUSOIDAL,         /* sin - unimplemented. */
    ORBITAL             /* orb - unimplemented. */
};

enum class TimeValueType {
    ARBITRARY,          /* Unitless abstract and arbitrary time values. */
    JULIAN_DAYS,        /* All times are in Julian Days J2000. */
    SECONDS             /* Unit is seconds. */
};

enum class PayoffModel {
    SIGMOID,            /* Sigmoid model (i.e. logitic function). */
    LINEAR              /* Linear normalisation with min-mid-max points.*/
};

enum class SandboxMode {
    SIMULATE,           /* Runs a simulation with the configured parameters. */
    TEST_PAYOFF         /* Enters a test mode and shows payoff values. */
};

#endif /* COMMON_ENUM_TYPES_HPP */
