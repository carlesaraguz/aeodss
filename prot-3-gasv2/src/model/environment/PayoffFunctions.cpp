/***********************************************************************************************//**
 *  Static class to provide various payoff functions.
 *  @class      PayoffFunctions
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-11
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "PayoffFunctions.hpp"
#include "Activity.hpp"

/*  REMARKS:
 *  ------------------------------------------------------------------------------------------------
 *  EnvCellPayoffFunc === Payoff function for one cell:
 *  Arg. #0:          shared_ptr<Activity>  --> The potential new activity.
 *  Arg. #1:            pair<float, float>  --> t0 & t1 of the potential new activity.
 *  Arg. #2:     vector<pair<float, float>  --> t0 & t1 of the activities for this cell.
 *  Arg. #3: vector<shared_ptr<Activity> >  --> Pointer to the activities (same index than arg2).
 *  Returns: the (potential) partial payoff value for this cell.
 *  ------------------------------------------------------------------------------------------------
 *  EnvCellCleanFunc === Cleaning function for one cell:
 *  Arg. #0:                         float  --> The current time.
 *  Arg. #1: vector<shared_ptr<Activity> >  --> Pointer to all the activities.
 *  Return: the list of activities that can be safely "forgotten" (i.e. removed).
 *  ------------------------------------------------------------------------------------------------
 **/

PayoffFunctions::FunctionPair PayoffFunctions::f_revisit_time_forwards;
PayoffFunctions::FunctionPair PayoffFunctions::f_revisit_time_backwards;
PayoffFunctions::FunctionPair PayoffFunctions::f_coverage;
PayoffFunctions::FunctionPair PayoffFunctions::f_latency;

void PayoffFunctions::bindPayoffFunctions(void)
{
    /* Revisit time forwards: ------------------------------------------------------------------- */
    f_revisit_time_forwards.first = [](PFArg0 /* a */, PFArg1 /* ats */, PFArg2 /* bts */, PFArg3 /* bs */) {
        return Random::getUf(0.f, 1.f);
    };
    f_revisit_time_forwards.second = [](CFArg0 /* t */, CFArg1 /* as */) {
        return CFArg1();
    };

    /* Revisit time backwards: ------------------------------------------------------------------ */
    f_revisit_time_forwards.first = [](PFArg0 /* a */, PFArg1 /* ats */, PFArg2 /* bts */, PFArg3 /* bs */) {
        return Random::getUf(0.f, 1.f);
    };
    f_revisit_time_forwards.second = [](CFArg0 /* t */, CFArg1 /* as */) {
        return CFArg1();
    };

    /* Coverage: -------------------------------------------------------------------------------- */
    f_revisit_time_forwards.first = [](PFArg0 /* a */, PFArg1 /* ats */, PFArg2 /* bts */, PFArg3 /* bs */) {
        return Random::getUf(0.f, 1.f);
    };
    f_revisit_time_forwards.second = [](CFArg0 /* t */, CFArg1 /* as */) {
        return CFArg1();
    };

    /* Latency: --------------------------------------------------------------------------------- */
    f_revisit_time_forwards.first = [](PFArg0 /* a */, PFArg1 /* ats */, PFArg2 /* bts */, PFArg3 /* bs */) {
        return Random::getUf(0.f, 1.f);
    };
    f_revisit_time_forwards.second = [](CFArg0 /* t */, CFArg1 /* as */) {
        return CFArg1();
    };
}
