/***********************************************************************************************//**
 *  Static class to provide various payoff functions.
 *  @class      PayoffFunctions
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-11
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef PAYOFF_FUNCTIONS_HPP
#define PAYOFF_FUNCTIONS_HPP

#include "prot.hpp"
#include "EnvCell.hpp"
#include "Random.hpp"

class Activity;

class PayoffFunctions
{
public:
    typedef std::pair<EnvCell::EnvCellPayoffFunc, EnvCell::EnvCellCleanFunc> FunctionPair;
    static FunctionPair f_revisit_time_forwards;
    static FunctionPair f_revisit_time_backwards;
    static FunctionPair f_coverage;
    static FunctionPair f_latency;

    static void bindPayoffFunctions(void);
    static float payoff(double rev_time);
private:
    typedef std::pair<double, double> PFArg0;
    typedef std::vector<std::vector<std::pair<double, double> > > PFArg1;
    typedef std::vector<std::shared_ptr<Activity> > PFArg2;
    typedef double CFArg0;
    typedef std::vector<std::shared_ptr<Activity> > CFArg1;

};

#endif /* PAYOFF_FUNCTIONS_HPP */
