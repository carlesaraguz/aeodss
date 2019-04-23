/***********************************************************************************************//**
 *  Task scheduler based on Genetic Algorithm.
 *  @class      GAScheduler
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-oct-08
 *  @version    0.2
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef GA_SCHEDULER_HPP
#define GA_SCHEDULER_HPP

#include "prot.hpp"
#include "Utils.hpp"
#include "Resource.hpp"
#include "GASChromosome.hpp"
#include "GASOperators.hpp"

class GAScheduler
{
public:
    /*******************************************************************************************//**
     *  Constructor.
     *  @param  t0      Start time (usually: VirtualTime::now()).
     *  @param  t1      End time of scheduling window.
     *  @param  res     Agent resources to consider for scheduling and fitness.
     **********************************************************************************************/
    GAScheduler(double t0, double t1, std::map<std::string, std::shared_ptr<const Resource> > res);

    /*******************************************************************************************//**
     *  Starts the GA Scheduler algorithm. Needs to (previously) having spwaned population and
     *  configured payoffs.
     *  @return Start and end times of scheduled activities and baseline confidence.
     **********************************************************************************************/
    std::vector<std::tuple<double, double, float> > schedule(void);

    /*******************************************************************************************//**
     *  Spawn population. Creates individuals (i.e. GASChromosome's) based on predefined activities/
     *  tasks and their information.
     *  @param  t0s     Start times for each activity/task.
     *  @param  s       Number of steps for each activity/task.
     *  @param  cs      Resources consumed by this activity (rates at each step).
     **********************************************************************************************/
    void setChromosomeInfo(std::vector<double> t0s, std::vector<int> s, const std::map<std::string, float>& cs);

    /*******************************************************************************************//**
     *  Prepares activity/choromosome information. Computes aggregated payoff for a given activity,
     *  based on the payoffs for each of their active cells.
     *  @param  idx     Activity index (i.e. chromosome allele).
     *  @param  cells   Cells affected or that would be captured during the activity `idx`.
     *  @param  payoff  Payoff values for these cells.
     *  @param  baseline_confidence The baseline confidence computed for this chromosome.
     **********************************************************************************************/
    void setAggregatedPayoff(unsigned int idx,
        const std::vector<sf::Vector2i>& cells,
        const std::vector<float>& payoff,
        float baseline_confidence);

    /*******************************************************************************************//**
     *  Shows debug information related to potential activities. Should allow finding the optimal
     *  solution manually and may assist in a debugging endeavor.
     **********************************************************************************************/
    void debug(void) const;

private:
    struct GASInfo {                            /**< Info for a single chromosome allele. */
        double t_start;                         /**< Start time of the allele. */
        int t_steps;                            /**< Steps of duration of this allele. */
        float ag_payoff;                        /**< Aggregated payoff that would be obtained. */
        float baseline_confidence;              /**< The baseline confidence for this chromosome. */
    };
    struct GASPrevSolution {
        double t_start;                         /**< Start time of the previously scheduled activity. */
        double t_end;                           /**< End time of the previously scheduled activity. */
        unsigned int a_start;                   /**< Index of starting allele (current chromosome). */
        unsigned int a_end;                     /**< Index of ending allele (current chrm.), included. */
        float confidence;                       /**< Confidence reported so far for this activity. */
    };
    const float m_big_coeff = 1e6f;             /**< Ensure big enough to discard resource violations. */
    const float m_small_coeff = 1e-4f;          /**< Ensure small. */

    float m_max_payoff;                         /**< The maximum payoff possible (i.e. payoff of a chromosome that enables all alleles). */
    std::map<std::string, float> m_max_cost;    /**< The total cost for each resource (i.e. cost of a chromosome that enables all alleles). */
    GASChromosome m_best;                       /**< A copy of the best chromosome for comparison purposes. */
    unsigned int m_generation_timeout;          /**< Counts number of generations where best has not changed. */
    std::vector<std::pair<unsigned int, float> > m_iteration_profile;   /**< Runtime information about the scheduling heuristic. */
    std::vector<GASChromosome> m_population;    /**< Chromosomes/individuals. */
    std::map<std::string, float> m_costs;       /**< Resource consumptions. */
    std::vector<GASInfo> m_individual_info;     /**< Info about chromosomes and their alleles. */
    double m_tstart;                            /**< Scheduling window start time. */
    double m_tend;                              /**< Scheduling window end time. */
    std::map<std::string, std::shared_ptr<const Resource> > m_resources_init;   /* Agent resources at start time. */

    /*******************************************************************************************//**
     *  Compute fitness of a chromosome. Takes payoffs for all the active alleles (i.e. tasks/
     *  activities) and divides by normalized resource states. Ensures that resource capacities are
     *  not exceeded but does not discard solutions where this happens. Instead, their payof is
     *  largely scaled down with a big constant.
     *  @param  c   The chromosome to compute the fitness of. Its fitness attribute will be set.
     *  @return     The chromosome fitness (which is also stored internally).
     **********************************************************************************************/
    float computeFitness(GASChromosome& c);

    /*******************************************************************************************//**
     *  Select a parent from the mating pool. Parent selection is based on either of the following
     *  techniques: tournament selection or fitness proportionate selection (a.k.a. roulette wheel).
     *  The selected parent will be removed from the pool.
     *  @param  mating_pool The pool of individuals where the parent will be selected and removed
     *                      from.
     *  @return A parent selected with the method in Config::ga_parentsel_op.
     **********************************************************************************************/
    GASChromosome select(std::vector<GASChromosome>& mating_pool) const;

    /*******************************************************************************************//**
     *  Repairs a pool of individuals by removing invalid ones.
     *  @param  pool    The pool to be removed.
     **********************************************************************************************/
    void repairPool(std::vector<GASChromosome>& pool);

    /*******************************************************************************************//**
     *  Combine two generations. Parents and children are combined based on the following
     *  techniques: truncation/elitist or generational. The result is stored in m_population.
     *  @param  parents     The parents.
     *  @param  children    Their offspring.
     *  @return The best individual after the combination.
     *  @note   Combination is performed according to the environment combination operator defined
     *          in Config::ga_environsel_op.
     **********************************************************************************************/
    GASChromosome combine(std::vector<GASChromosome> parents, std::vector<GASChromosome> children);

    /*******************************************************************************************//**
     *  Determines whether to continue iterating (i.e. trying to improve solution) or not.
     *  @param  g       Current number of generations/iterations.
     *  @param  best    Current best individual/solution.
     *  @return         True if GA scheduler needs to keep looking for solutions. False otherwise.
     **********************************************************************************************/
    bool iterate(unsigned int& g, GASChromosome best);
};


#endif /* GA_SCHEDULER_HPP */
