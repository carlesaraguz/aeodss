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
class Agent;

struct EnvCellState {
    double* t0s;        /* Times when an activity starts influencing. */
    double* t1s;        /* Times when an activity ends influencing. */
    int nts;            /* Number of times that an activity influences over this cell. */
};

class EnvCell
{
public:
    /*  Payoff function for one cell:
     *  Arg. #0:                   pair<double, double>  --> t0 & t1 of the potential new activity.
     *  Arg. #1: vector<vector<pair<double, double> > >  --> vector of t0 & t1 of the activities for this cell.
     *  Arg. #2:          vector<shared_ptr<Activity> >  --> Pointer to the activities (same index than arg2).
     *
     *  Return: a pair of floats:
     *      first  -> the (potential) partial payoff value for this cell.
     *      second -> the sum of confidence utilities that contributed to the payoff.
     **/
    typedef std::function<std::pair<float, float>(
        std::pair<double, double>,
        std::vector<std::vector<std::pair<double, double> > >,
        std::vector<std::shared_ptr<Activity> >
    )> EnvCellPayoffFunc;

    /*  Cleaning function for one cell:
     *  Arg. #0:                        double  --> The current time.
     *  Arg. #1: vector<shared_ptr<Activity> >  --> Pointer to all the activities.
     *
     *  Return: the list of activities that can be safely "forgotten" (i.e. removed).
     **/
    typedef std::function<std::vector<std::shared_ptr<Activity> >(
        double,
        std::vector<std::shared_ptr<Activity> >
    )> EnvCellCleanFunc;

    unsigned int x;
    unsigned int y;

    EnvCell(Agent* agnt, unsigned int cx, unsigned int cy);
    EnvCell(Agent* agnt, unsigned int cx, unsigned int cy, EnvCellPayoffFunc fp, EnvCellCleanFunc fc);

    void addCellActivity(std::shared_ptr<Activity> aptr);
    bool removeCellActivity(std::shared_ptr<Activity> aptr);
    bool removeCellActivityById(std::string agent_id, unsigned int activity_id);
    bool updateCellActivity(std::shared_ptr<Activity> aptr);
    std::vector<std::shared_ptr<Activity> > getAllActivities(void) const;
    float computeCellPayoff(double* at0s, double* at1s, int nts);
    void clean(double t);
    std::size_t pushPayoffFunc(const EnvCellPayoffFunc fp, const EnvCellCleanFunc fc);
    std::size_t pushPayoffFunc(const std::pair<EnvCellPayoffFunc, EnvCellCleanFunc> f);
    std::size_t getPayoffFuncCount(void) { return m_payoff_func.size(); }
    void getPayoff(double t, float& payoff, float& utility) const;
    std::map<double, std::pair<float, float> > getAllPayoffs(void) const { return m_payoff; }
    std::size_t getPayoffCount(void) const { return m_payoff.size(); }

    /* Friend debug functions: */
    friend std::ostream& operator<<(std::ostream& os, const EnvCell& ec);

private:
    Agent* m_agent;
    std::map<std::shared_ptr<Activity>, EnvCellState> m_activities;
    std::vector<EnvCellPayoffFunc> m_payoff_func;
    std::vector<EnvCellCleanFunc> m_clean_func;
    std::map<double, std::pair<float, float> > m_payoff;    /**< Time of payoff <-> {Payoff value, Avg. utility}. */
};

#endif /* ENV_CELL_HPP */
