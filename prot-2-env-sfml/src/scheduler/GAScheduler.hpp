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
#include "common_enum_types.hpp"
#include "Intent.hpp"
#include "Utils.hpp"
#include "GASChromosome.hpp"
#include "GASReward.hpp"
#include "GASOperators.hpp"

class GAScheduler
{
public:
    GAScheduler(void);

    void initPopulation(std::vector<Intent> prev_res = std::vector<Intent>());
    void setSchedulingWindow(unsigned int span, const std::vector<float>& ts);
    void setRewards(std::vector<std::shared_ptr<GASReward> > rptrs,
        std::vector<std::vector<std::size_t> > rptrs_lut,
        Aggregate ag_type,
        float min_dist);
    std::vector<Intent> schedule(bool verbose = false);

    void setInitResource(float r) { m_init_resource = r; }

private:
    std::vector<GASChromosome> m_population;                /**< Bag of solutions.                  */
    float m_init_resource;                                  /**< Initial resource state.            */
    unsigned int m_sched_win_span;                          /**< Scheduling window length.          */
    std::vector<std::shared_ptr<GASReward> > m_rewards;     /**< Reward objects.                    */
    Aggregate m_aggregation_type;                           /**< Reward aggregation method.         */
    int m_reward_step;                                      /**< Min. steps to re-compute reward.   */

    /* Look-up tables ---- element access key = time index. */
    std::vector<float> m_time_lut;
    std::vector<std::vector<std::size_t> > m_reward_lut;    /**< For each element there is a number
                                                             *   of GASReward's to check.
                                                             **/

    float computeConsumption(const GASChromosome& ind) const;
    void computeFitness(GASChromosome& ind, float rnorm_factor);
    void computeFitnessParallel(std::vector<GASChromosome>& children, float rnf);
    void computeFitnessHelper(std::vector<GASChromosome>::iterator c0, std::vector<GASChromosome>::iterator c1, float rnf);
    bool satisfiesConstraints(GASChromosome ind, bool show = false) const;
    GASChromosome selectParent(std::vector<GASChromosome>& mating_pool) const;
    GASChromosome combine(std::vector<GASChromosome> parents, std::vector<GASChromosome> children);
    bool stopGeneration(unsigned int gencount, float prev_fitmax, float fitmax) const;
};

#endif /* GA_SCHEDULER_HPP */
