/***********************************************************************************************//**
 *  Task scheduler based on Genetic Algorithm.
 *  @class      GAScheduler
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-11
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "GAScheduler.hpp"

GAScheduler::GAScheduler(void)
    : m_sched_win_span(0)
    , m_init_resource(0.f)
{
    m_population.reserve(Config::ga_population_size);
}

void GAScheduler::setRewards(
    std::vector<std::shared_ptr<GASReward> > rptrs,
    std::vector<std::vector<std::size_t> > rptrs_lut,
    Aggregate ag_type,
    float min_dist
)
{
    if((rptrs_lut.size() - 1) != m_sched_win_span) {
        std::cerr << "Genetic Algorithm Scheduler Error: setting rewards failed. " << (rptrs_lut.size() - 1) << " != " << m_sched_win_span << "\n";
        std::exit(-1);
    }
    m_rewards     = rptrs;
    m_reward_lut  = rptrs_lut;
    m_reward_step = std::max(1, (int)std::ceil(min_dist / Config::agent_speed / Config::time_step));
    m_aggregation_type = ag_type;
}

void GAScheduler::setSchedulingWindow(unsigned int span, const std::vector<float>& ts)
{
    m_time_lut = ts;
    m_sched_win_span = span;
}

void GAScheduler::initPopulation(std::vector<Intent> prev_res)
{
    m_population.clear();
    if(prev_res.size() == 0 || prev_res.size() > Config::max_tasks) {
        for(unsigned int i = 0; i < Config::ga_population_size; i++) {
            m_population.push_back(GASChromosome(m_sched_win_span, m_sched_win_span));
        }
    } else {
        for(unsigned int i = 0; i < Config::ga_population_size; i++) {
            m_population.push_back(GASChromosome(m_sched_win_span, m_sched_win_span, prev_res));
        }
    }
}

bool GAScheduler::stopGeneration(unsigned int gencount, float /* prev_fitmax */, float /* fitmax */) const
{
    if(gencount < Config::ga_generations) {
        return false;
    } else {
        return true;
    }
}

std::vector<Intent> GAScheduler::schedule(bool verbose)
{
    GASChromosome best;
    float prev_fit_max = 0.f, fit_max = 0.f;
    float rmax = 0.f;

    unsigned int generation_count = 0;
    while(!stopGeneration(generation_count++, prev_fit_max, fit_max)) {
        std::vector<GASChromosome> children;
        std::vector<GASChromosome> parents = m_population;  /* Copy. */
        while(children.size() < m_population.size()) {
            GASChromosome parent1 = selectParent(parents);
            GASChromosome parent2 = selectParent(parents);
            GASChromosome child1;
            GASChromosome child2;
            GASChromosome::crossover(parent1, parent2, child1, child2);
            child1.mutate();
            child2.mutate();
            child1.repair();
            child2.repair();
            rmax = std::max(computeConsumption(child1), rmax);
            rmax = std::max(computeConsumption(child2), rmax);
            children.push_back(child1);
            children.push_back(child2);
        }
        computeFitnessParallel(children, rmax);
        best = combine(m_population, children);    /* Updates m_population. */
        prev_fit_max = fit_max;
        fit_max = best.getFitness();

        if(verbose) {
            if( (generation_count %   5 == 0 && generation_count < Config::ga_generations / 100) ||
                (generation_count %  50 == 0 && generation_count < Config::ga_generations / 10 ) ||
                (generation_count % 100 == 0)) {
                std::clog << "Genetic Algorithm Scheduler running... Fitness: "
                    << std::fixed << std::setprecision(3) << std::setw(8) << fit_max << ":: "
                    << "(" << best.getActiveSlotCount() << " activities) Progress: ";
                std::clog << std::fixed << std::setprecision(1) << 50.f * (float)generation_count / Config::ga_generations
                    << "%\r" << std::flush;
            }

        }
    }
    if(verbose) {
        std::clog << "\n";
    }

    auto retval = std::vector<Intent>();
    if(satisfiesConstraints(best)) {
        for(unsigned int iid = 0; iid < best.getTaskCount(); iid++) {
            if(best.isEnabled(iid)) {
                /* Constructs Intent only providing tstart and tend: */
                retval.emplace(
                    retval.end(),
                    m_time_lut[best.getStart(iid)],
                    m_time_lut[best.getStart(iid) + best.getDuration(iid)]
                );
            }
        }
    } else {
        std::cerr << "Genetic Algorithm Scheduler Error: the best solution does not satisfy resource constraints:" << best;
    }
    return retval;
}

float GAScheduler::computeConsumption(const GASChromosome& ind) const
{
    float acc = 0.f;
    float duration_time;
    for(unsigned int task = 0; task < ind.getTaskCount(); task++) {
        if(ind.isEnabled(task)) {
            duration_time  = m_time_lut[ind.getStart(task) + ind.getDuration(task)];
            duration_time -= m_time_lut[ind.getDuration(task)];
            acc += duration_time * Config::capacity_consume;
        }
    }
    return acc;
}

void GAScheduler::computeFitnessParallel(std::vector<GASChromosome>& children, float rnf)
{
    unsigned int n_children = children.size() / Config::ga_thread_pool_size;
    std::vector<std::thread> thread_pool;
    for(unsigned int thread_id = 0; thread_id < Config::ga_thread_pool_size; thread_id++) {
        auto c0 = children.begin() + n_children * thread_id;
        auto c1 = children.begin() + n_children * (thread_id + 1);
        if(thread_id == Config::ga_thread_pool_size - 1) {
            c1 = children.end();
        }
        thread_pool.push_back(std::thread(&GAScheduler::computeFitnessHelper, this, c0, c1, rnf));
    }
    for(auto& th : thread_pool) {
        th.join();
    }
}

void GAScheduler::computeFitnessHelper(std::vector<GASChromosome>::iterator c0, std::vector<GASChromosome>::iterator c1, float rnf)
{
    for(auto c = c0; c != c1; c++) {
        computeFitness(*c, rnf);
    }
}

void GAScheduler::computeFitness(GASChromosome& ind, float rnorm_factor)
{
    float modifier;
    float fitness = 0.f;
    float acc_resources = 0.f;
    float duration_time;

    std::vector<bool> m_consumed_mask(m_rewards.size(), false);

    /*  Get a sorted list of enabled tasks as an ordered map of task durations with its key being
     *  the task start time:
     **/
    std::map<unsigned int, unsigned int> tasks;     /* pair: start time, duration. */
    for(unsigned int task = 0; task < ind.getTaskCount(); task++) {
        if(ind.isEnabled(task)) {
            /* (A) Fill-in temporary map: */
            tasks[ind.getStart(task)] = ind.getDuration(task);

            /* (B) Accumulate resource consumption: */
            duration_time = m_time_lut[ind.getStart(task) + ind.getDuration(task)] - m_time_lut[ind.getStart(task)];
            acc_resources += duration_time * Config::capacity_consume + Config::task_startup_cost;
        }
    }
    acc_resources = 1.f - (acc_resources / rnorm_factor);

    /*  Compute fitness for each task based on rewards:
     *  Note that this process expects/assumes that these `tasks` are not overlapping and are sorted
     *  early-to-later. Sorting is, a priori, guaranteed by the ordedness of std::map.
     **/
    int rcount = 0;
    for(const auto& t : tasks) {
        unsigned int i;
        float reward_at_i = 0.f;
        for(i = t.first; i < t.first + t.second; i += m_reward_step) {
            reward_at_i = 0.f;
            for(std::size_t j = 0; j < m_reward_lut[i].size(); j += 1) {
                if(!m_consumed_mask[m_reward_lut[i][j]]) {
                    switch(m_aggregation_type) {
                        case Aggregate::MAX_VALUE:
                            reward_at_i = std::max(m_rewards[m_reward_lut[i][j]]->consumeReward(i), reward_at_i);
                            break;
                        case Aggregate::MIN_VALUE:
                            reward_at_i = std::min(m_rewards[m_reward_lut[i][j]]->consumeReward(i), reward_at_i);
                            break;
                        case Aggregate::MEAN_VALUE:
                            reward_at_i += m_rewards[m_reward_lut[i][j]]->consumeReward(i);
                            rcount++;
                            break;
                    }
                    m_consumed_mask[m_reward_lut[i][j]] = true;
                }
                // std::cout << "/* message */2" << '\n';
            }
            if(m_aggregation_type == Aggregate::MEAN_VALUE) {
                reward_at_i /= (float)rcount;
            }
            fitness += reward_at_i;
        }
        /* Ensure reward is also computed at the end: */
        if(i != t.first + t.second - 1) {
            i = t.first + t.second - 1;
            for(std::size_t j = 0; j < m_reward_lut[i].size(); j += 1) {
                if(!m_consumed_mask[m_reward_lut[i][j]]) {
                    switch(m_aggregation_type) {
                        case Aggregate::MAX_VALUE:
                            reward_at_i = std::max(m_rewards[m_reward_lut[i][j]]->consumeReward(i), reward_at_i);
                            break;
                        case Aggregate::MIN_VALUE:
                            reward_at_i = std::min(m_rewards[m_reward_lut[i][j]]->consumeReward(i), reward_at_i);
                            break;
                        case Aggregate::MEAN_VALUE:
                            reward_at_i += m_rewards[m_reward_lut[i][j]]->consumeReward(i);
                            rcount++;
                            break;
                    }
                }
            }
            if(m_aggregation_type == Aggregate::MEAN_VALUE) {
                reward_at_i /= (float)rcount;
            }
            fitness += reward_at_i;
        }
    }

    /*  Check resource constrint satisfaction and set the fitness modifier value accordingly: */
    if(satisfiesConstraints(ind)) {
        modifier = 1.f;
    } else {
        modifier = 1e-5f;
    }
    ind.setFitness(fitness * acc_resources * modifier);
}

bool GAScheduler::satisfiesConstraints(GASChromosome ind, bool show) const
{
    std::vector<float> t0s;
    std::vector<float> t1s;
    for(unsigned int i = 0; i < ind.getTaskCount(); i++) {
        if(ind.isEnabled(i)) {
            t0s.push_back(m_time_lut[ind.getStart(i)]);
            t1s.push_back(m_time_lut[ind.getStart(i) + ind.getDuration(i)]);
        }
    }

    if(show) {
        for(std::size_t i = 0; i < t0s.size(); i++) {
            std::clog << "**************************** (" << i << ") Task: " << t0s[i] << " ··· " << t1s[i]
                << "; Duration: " << t1s[i] - t0s[i]
                << "; Consumption: "  << (t1s[i] - t0s[i]) * Config::capacity_consume << "\n";
        }
        std::clog << "** Init resource: " << m_init_resource << "\n";
        std::clog << "** Start time:    " << m_time_lut[0] << "\n";
        std::clog << "---------------------------\n";
    }

    bool retval = true;
    float dr;
    float r = m_init_resource;
    float t = m_time_lut[0];
    for(std::size_t i = 0; i < t0s.size(); i++) {
        dr = Config::capacity_restore;
        if(r + dr * (t0s[i] - t) >= Config::max_capacity) {
            r  = Config::max_capacity;
        } else {
            r += dr * (t0s[i] - t);
        }
        if(show) {
            std::clog << "** At T0 = " << t0s[i] << ": R = " << r << "\n";
        }
        dr = Config::capacity_restore - Config::capacity_consume;
        r -= Config::task_startup_cost;
        r += dr * (t1s[i] - t0s[i]);
        t  = t1s[i];
        if(show) {
            std::clog << "** At T1 = " << t1s[i] << ": R = " << r << "\n";
            std::clog << "---------------------------\n";
        }
        if(r < Config::capacity_consume) {
            return false;
        }
    }
    return retval;
}

GASChromosome GAScheduler::selectParent(std::vector<GASChromosome>& mating_pool) const
{
    GASChromosome retval; /* Initialises at random and with all fields = 0. */

    switch(Config::ga_parentsel_op) {
        case GASSelectionOp::TOURNAMENT:
            {
                bool bflag = false;
                for(unsigned int k = 0; k < Config::ga_tournament_k; k++) {
                    GASChromosome ind = mating_pool[Random::getUi(0, mating_pool.size() - 1)];
                    if(!bflag || ind > retval) {
                        retval = ind;
                    }
                }
                mating_pool.erase(std::find(mating_pool.begin(), mating_pool.end(), retval));
            }
            break;
        case GASSelectionOp::FITNESS_PROPORTIONAL_ROULETTE_WHEEL:
        case GASSelectionOp::STOCHASTIC_UNIVERSAL:
        case GASSelectionOp::ELITIST:
            std::clog << "Genetic Algorithm failure. Unimplemented operator (P selection).\n";
            break;
        case GASSelectionOp::TRUNCATION:
        case GASSelectionOp::GENERATIONAL:
            std::clog << "Genetic Algorithm failure. Truncation and Generational selection operators ";
            std::clog << "are not suitable for parent selection.\n";
            break;
    }
    return retval;
}

GASChromosome GAScheduler::combine(std::vector<GASChromosome> parents, std::vector<GASChromosome> children)
{
    GASChromosome best_individual;
    switch(Config::ga_environsel_op) {
        case GASSelectionOp::TRUNCATION:
        case GASSelectionOp::ELITIST:
            {
                auto pc = parents;
                pc.insert(pc.end(), children.begin(), children.end());
                std::sort(pc.begin(), pc.end(), std::greater<GASChromosome>());
                while(pc.size() > Config::ga_population_size) {
                    pc.pop_back();
                }
                m_population = pc;
                best_individual = m_population[0];
            }
            break;
        case GASSelectionOp::GENERATIONAL:
            m_population = children;
            std::sort(m_population.begin(), m_population.end(), std::greater<GASChromosome>());
            best_individual = m_population[0];
            break;
        case GASSelectionOp::TOURNAMENT:
        case GASSelectionOp::FITNESS_PROPORTIONAL_ROULETTE_WHEEL:
        case GASSelectionOp::STOCHASTIC_UNIVERSAL:
            std::clog << "Genetic Algorithm failure. Unimplemented operator (P-C combination).\n";
            break;
    }
    return best_individual;
}
