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
    void setSchedulingWindow(unsigned int span, const std::vector<float>& ts);
    void setRewards(std::vector<std::vector<std::tuple<unsigned int, float> > > rewards);
    std::vector<Intent> schedule(void);

    void setInitResource(float r) { m_init_resource = r; }

private:
    std::vector<GASChromosome> m_population;
    std::vector<GASReward> m_rewards;
    std::vector<float> m_time_lut;
    std::map<unsigned int, bool> m_cells_lut;
    float m_init_resource;
    unsigned int m_sched_win_span;

    void resetCellLUT(void);
    float computeConsumption(const GASChromosome& ind) const;
    void computeFitness(GASChromosome& ind, float rnorm_factor);
    bool satisfiesConstraints(GASChromosome ind) const;
    GASChromosome selectParent(std::vector<GASChromosome>& mating_pool) const;
    GASChromosome combine(std::vector<GASChromosome> parents, std::vector<GASChromosome> children);
    bool stopGeneration(unsigned int gencount, float prev_fitmax, float fitmax) const;
};

#endif /* GA_SCHEDULER_HPP */
