/***********************************************************************************************//**
 *  Task scheduler based on Genetic Algorithm.
 *  @class      GAScheduler
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-11
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef GA_SCHEDULER_HPP
#define GA_SCHEDULER_HPP

#include "prot.hpp"
#include "Intent.hpp"
#include "GASChromosome.hpp"
#include "GASReward.hpp"
#include "GASOperators.hpp"

class GAScheduler
{
public:
    GAScheduler(void);

    void initPopulation(std::vector<Intent> prev_res = std::vector<Intent>());
    void setRewards(std::vector<float> rewards);
    void setInitResource(float r) { m_init_resource = r; }
    void setSchedulingWindow(float t0, float t1);
    std::vector<Intent> schedule(void);

private:
    std::vector<GASChromosome> m_population;
    std::vector<GASReward> m_rewards;
    float m_init_resource;
    float m_sched_win_start;
    float m_sched_win_end;

    float computeConsumption(const GASChromosome& ind) const;
    void computeFitness(GASChromosome& ind, float rnorm_factor);
    bool satisfiesConstraints(GASChromosome ind, float* exceeded_res = nullptr) const;
    GASChromosome selectParent(std::vector<GASChromosome>& mating_pool) const;
    GASChromosome combine(std::vector<GASChromosome> parents, std::vector<GASChromosome> children);
    bool stopGeneration(unsigned int gencount, float prev_fitmax, float fitmax, float fitmin) const;
};

#endif /* GA_SCHEDULER_HPP */
