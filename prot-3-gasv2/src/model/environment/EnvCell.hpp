/***********************************************************************************************//**
 *  A single spatial segment of the environment.
 *  @class      EnvCell
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-10
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef ENV_CELL_HPP
#define ENV_CELL_HPP

#include "prot.hpp"

class Activity;

struct EnvCellState {
    float t0;       /* Time when this activity starts influencing. */
    float t1;       /* Time when this activity ends influencing. */
};

class EnvCell
{
public:
    /*  Payoff function for one cell:
     *  Arg. #0:          shared_ptr<Activity>  --> The potential new activity.
     *  Arg. #1:            pair<float, float>  --> t0 & t1 of the potential new activity.
     *  Arg. #2:     vector<pair<float, float>  --> t0 & t1 of the activities for this cell.
     *  Arg. #3: vector<shared_ptr<Activity> >  --> Pointer to the activities (same index than arg2).
     *
     *  Return: the (potential) partial payoff value for this cell.
     **/
    typedef std::function<float(
        std::shared_ptr<Activity>,
        std::pair<float, float>,
        std::vector<std::pair<float, float> >,
        std::vector<std::shared_ptr<Activity> >
    )> EnvCellPayoffFunc;

    /*  Cleaning function for one cell:
     *  Arg. #0:                         float  --> The current time.
     *  Arg. #1: vector<shared_ptr<Activity> >  --> Pointer to all the activities.
     *
     *  Return: the list of activities that can be safely "forgotten" (i.e. removed).
     **/
    typedef std::function<std::vector<std::shared_ptr<Activity> >(
        float,
        std::vector<std::shared_ptr<Activity> >
    )> EnvCellCleanFunc;

    unsigned int x;
    unsigned int y;

    EnvCell(unsigned int cx, unsigned int cy);
    EnvCell(unsigned int cx, unsigned int cy, EnvCellPayoffFunc fp, EnvCellCleanFunc fc);

    void addActivity(std::shared_ptr<Activity> aptr);
    void removeActivity(std::shared_ptr<Activity> aptr);
    std::vector<std::shared_ptr<Activity> > getAllActivities(void) const;
    float computeCellPayoff(unsigned int fidx, std::shared_ptr<Activity> aptr) const;
    void clean(unsigned int fidx, float t);
    std::size_t pushPayoffFunc(const EnvCellPayoffFunc fp, const EnvCellCleanFunc fc);
    std::size_t pushPayoffFunc(const std::pair<EnvCellPayoffFunc, EnvCellCleanFunc> f);
    std::size_t getPayoffCount(void) const { return m_payoff_func.size(); }

private:
    std::map<std::shared_ptr<Activity>, EnvCellState> m_activities;
    std::vector<EnvCellPayoffFunc> m_payoff_func;
    std::vector<EnvCellCleanFunc> m_clean_func;
};

#endif /* ENV_CELL_HPP */
