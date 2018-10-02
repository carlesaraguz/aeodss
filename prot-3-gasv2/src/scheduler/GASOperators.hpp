/***********************************************************************************************//**
 *  Genetic Algorithm Scheduler operator types.
 *  @class      GASSelectionOp
 *  @class      GASCrossoverOp
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-16
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef GAS_OPERATORS_HPP
#define GAS_OPERATORS_HPP

#include "prot.hpp"

enum class GASSelectionOp {
    FITNESS_PROPORTIONAL_ROULETTE_WHEEL,
    STOCHASTIC_UNIVERSAL,
    TRUNCATION,
    TOURNAMENT,
    ELITIST,
    GENERATIONAL
};

enum class GASCrossoverOp {
    SINGLE_POINT,
    MULIPLE_POINT,
    UNIFORM
};

#endif /* GAS_OPERATORS_HPP */
