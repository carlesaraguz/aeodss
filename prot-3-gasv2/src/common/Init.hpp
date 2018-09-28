/***********************************************************************************************//**
 *  Program initialization and setup class.
 *  @class      Init
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef INIT_HPP
#define INIT_HPP

#include "prot.hpp"
#include "Random.hpp"
#include "Utils.hpp"
#include "ColorGradient.hpp"
#include "PayoffFunctions.hpp"
// #include "GAScheduler.hpp"

class Init
{
public:
    static void doInit(void);
private:
    static std::string getRootPath(void);
};

#endif /* INIT_HPP */
