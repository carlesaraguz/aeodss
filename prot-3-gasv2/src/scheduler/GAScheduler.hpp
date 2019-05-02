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
#include "Activity.hpp"

class GAScheduler
{
public:
    /*******************************************************************************************//**
     *  (1) double: start time of a new task.
     *  (2) double: end time of a new task.
     *  (3) float: baseline confidence of the new task.
     **********************************************************************************************/
    typedef std::vector<std::tuple<double, double, float> > Solution;

    /*******************************************************************************************//**
     *  Constructor.
     *  @param  t0      Start time (usually: VirtualTime::now()).
     *  @param  t1      End time of scheduling window.
     *  @param  res     Agent resources to consider for scheduling and fitness.
     **********************************************************************************************/
    GAScheduler(double t0, double t1, std::map<std::string, std::shared_ptr<const Resource> > res);

    /*******************************************************************************************//**
     *  Starts the GA Scheduler algorithm. Before calling this function the population has to be
     *  spawned (with setChromosomeInfo) and the payoff information has to be configured (with
     *  setAggregatedPayoff). Note that previously scheduled activities that are no longer part of
     *  the solution, are not discarded in this routine. They are added to adis and are a return
     *  value that should be used by the agent.
     *  @param  adis    List of discarded activities.
     *  @return         Start and end times of scheduled activities and baseline confidence.
     **********************************************************************************************/
    Solution schedule(std::vector<std::shared_ptr<Activity> >& adis);

    /*******************************************************************************************//**
     *  Spawn population. Creates individuals (i.e. GASChromosome's) based on predefined activities/
     *  tasks and their information.
     *  @param  t0s     Start times for each activity/task/allele.
     *  @param  t1s     End times for each activity/task/allele.
     *  @param  cs      Resources consumed by this activity (rates at each step).
     **********************************************************************************************/
    void setChromosomeInfo(std::vector<double> t0s, std::vector<double> t1s, const std::map<std::string, float>& cs);

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
     *  Defines a chromosome range to belong to a previous scheduling solution (i.e. an existing
     *  activity).
     *  @param  a_start     The index of the allele where this task starts.
     *  @param  a_end       The index of the allele where this task ends (inclusive). If it only
     *                      takes one allele, then a_start = a_end.
     *  @param  aptr        The actual activity.
     **********************************************************************************************/
    void setPreviousSolution(unsigned int a_start, unsigned int a_end, std::shared_ptr<Activity> aptr);

    /*******************************************************************************************//**
     *  Shows debug information related to potential activities. Should allow finding the optimal
     *  solution manually and may assist in a debugging endeavor.
     **********************************************************************************************/
    void debug(void) const;

private:
    struct GASInfo {                            /**< Info for a single chromosome allele. */
        double t_start;                         /**< Start time of the allele. */
        double t_end;                           /**< End time of the allele. */
        // int t_steps;                            /**< Steps of duration of this allele. */
        float ag_payoff;                        /**< Aggregated payoff that would be obtained. */
        float baseline_confidence;              /**< The baseline confidence for this chromosome. */
    };
    struct GASPrevSolution {
        unsigned int a_start;                   /**< Index of starting allele (current chromosome). */
        unsigned int a_end;                     /**< Index of ending allele (current chrm.), included. */
        std::shared_ptr<Activity> activity;     /**< The pointer to the actual activity. */
        float lambda;                           /**< The payoff augmentation factor. */
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
    std::vector<GASPrevSolution> m_previous_solutions;  /* Infor about previous solutions that have been encoded in the chromosome. */
    double m_tstart;                            /**< Scheduling window start time. */
    double m_tend;                              /**< Scheduling window end time. */
    std::map<std::string, std::shared_ptr<const Resource> > m_resources_init;   /* Agent resources at start time. */
    GASChromosome m_init_individual;            /**< An individual that has protected alleles to initialise others. */

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
     *  @param  pool    The pool to removed the invalid solutions from.
     *  @return         The number of invalid solutions.
     **********************************************************************************************/
    int repairPool(std::vector<GASChromosome>& pool);

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
     *  Protects chromosome values for activities that are belong to previous solutions and are
     *  confirmed. This should be called once after the initial population has been generated.
     **********************************************************************************************/
    void protectPopulation(void);

    /*******************************************************************************************//**
     *  Generates a solution from chromosome `c` and identifies activities that are no longer part
     *  of the solution (and hence need be discarded by their owning agent).
     *  @param  c       The best individual/solution.
     *  @param  adis    A reference to a list of discarded activities (will be cleared at the
     *                  beginning).
     *  @return         Set of values that are needed to create NEW activities.
     **********************************************************************************************/
    Solution generateSolution(GASChromosome& c, std::vector<std::shared_ptr<Activity> >& adis);

    /*******************************************************************************************//**
     *  Determines whether to continue iterating (i.e. trying to improve solution) or not.
     *  @param  g       Current number of generations/iterations.
     *  @param  best    Current best individual/solution.
     *  @return         True if GA scheduler needs to keep looking for solutions. False otherwise.
     **********************************************************************************************/
    bool iterate(unsigned int& g, GASChromosome best);
};


#endif /* GA_SCHEDULER_HPP */
