/***********************************************************************************************//**
 *  Task scheduler based on Genetic Algorithm.
 *  @class      GAScheduler
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-oct-08
 *  @version    0.2
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "GAScheduler.hpp"

CREATE_LOGGER(GAScheduler)

GAScheduler::GAScheduler(float t0, float t1, std::map<std::string, std::shared_ptr<const Resource> > res)
    : m_tstart(t0)
    , m_tend(t1)
    , m_resources_init(res)
{
    m_population.reserve(Config::ga_population_size);
}

bool GAScheduler::iterate(unsigned int& g, float f)
{
    bool do_continue = true;
    if(m_iteration_profile.size() == 0) {
        m_iteration_profile.push_back(std::make_pair(g, f));
    }
    if(g >= Config::ga_generations) {
        do_continue = false;
        Log::dbg << "GA Scheduler reached the maximum amount of generations, stopping now.\n";
    } else if(g > Config::ga_generations / 2) {
        /* Check improvement: */
        unsigned int g0 = m_iteration_profile[m_iteration_profile.size() - 1].first;
        float f0        = m_iteration_profile[m_iteration_profile.size() - 1].second;
        if(std::abs(f - f0) > 1e-8f) {
            float delta = (f - f0) / (g - g0);
            Log::dbg << "GA Scheduler, at g = " << g << ": f = " << f << ", Δ(abs) = " << std::abs(f - f0) << ", Δ(rel) = " << delta << "\n";
            m_iteration_profile.push_back(std::make_pair(g, f));
            if(delta < Config::ga_min_improvement_rate) {
                do_continue = false;
            }
        }
        g++;
    } else {
        // m_iteration_profile[0].first  = g;
        // m_iteration_profile[0].second = f;
        g++;
    }
    return do_continue;
}

std::vector<std::pair<float, float> > GAScheduler::schedule(void)
{
    if(m_population.size() == 0) {
        Log::err << "Cannot start scheduling before population has been spawned.\n";
        throw std::runtime_error("GA Scheduler failed to start because population is not ready.");
    }

    GASChromosome best(m_individual_info.size());  /* Randomly initializes. */
    float best_f = 0.f;
    unsigned int g = 0;
    m_iteration_profile.clear();
    while(iterate(g, best_f)) {
        std::vector<GASChromosome> children;
        std::vector<GASChromosome> parents = m_population;  /* Copy. */
        while(children.size() < m_population.size()) {
            GASChromosome parent1 = select(parents);
            GASChromosome parent2 = select(parents);
            GASChromosome child1(m_individual_info.size());
            GASChromosome child2(m_individual_info.size());
            GASChromosome::crossover(parent1, parent2, child1, child2);
            child1.mutate();
            child2.mutate();
            children.push_back(child1);
            children.push_back(child2);
        }
        #pragma omp parallel for
        for(unsigned int i = 0; i < children.size(); i++) {
            computeFitness(children[i]);
        }
        best = combine(m_population, children);    /* Environment selection: updates population. */
        best_f = best.fitness;
        // Log::dbg << "GA Scheduler, best: " << best << ".\n";
    }

    std::vector<std::pair<float, float> > retvec;
    if(best.valid) {
        Log::dbg << "GA Scheduler completed after " << g << " iterations. Solution:\n";
        float t0, t1;
        bool bflag = false;
        for(unsigned int i = 0; i < best.alleles.size(); i++) {
            if(best.alleles[i] && !bflag) {
                /* Start a new activity: */
                t0 = m_individual_info[i].t_start;
                t1 = m_individual_info[i].t_start + m_individual_info[i].t_steps * Config::time_step;
                bflag = true;
            } else if(best.alleles[i] && bflag) {
                /* Continue/extend a previous activity: */
                t1 += m_individual_info[i].t_steps * Config::time_step;
            } else if(!best.alleles[i] && bflag) {
                /* Record the activity: */
                retvec.push_back(std::make_pair(t0, t1));
                Log::dbg << " * Activity " << (retvec.size() - 1) << ": [" << t0 << ", " << t1 << ").\n";
                bflag = false;
            }
        }
        if(bflag) {
            /* Record the last activity: */
            retvec.push_back(std::make_pair(t0, t1));
            Log::dbg << " * Activity " << (retvec.size() - 1) << ": [" << t0 << ", " << t1 << ").\n";
        }
    } else {
        Log::warn << "GA Scheduler completed after " << g << " iterations, but could not find a solution.\n";
    }
    return retvec;
}

void GAScheduler::setChromosomeInfo(std::vector<float> t0s, std::vector<int> s, const std::map<std::string, float>& cs)
{
    if(m_population.size() > 0) {
        std::vector<GASChromosome> other;
        other.reserve(Config::ga_population_size);
        m_population.swap(other);
    }

    if(m_individual_info.size() > 0) {
        std::vector<GASInfo> info_vec;
        m_individual_info.swap(info_vec);
    }

    unsigned int l;
    if(t0s.size() != s.size()) {
        Log::err << "Chromosome start times and number of steps mismatch. Aborting setup.\n";
        return;
    } else {
        l = t0s.size();
        m_individual_info.reserve(l);
        for(unsigned int i = 0; i < l; i++) {
            GASInfo gas_info;
            gas_info.t_start = t0s[i];
            gas_info.t_steps = s[i];
            m_individual_info.push_back(gas_info);
        }
    }

    bool flag = false;
    for(auto& r : cs) {
        if(m_resources_init.find(r.first) == m_resources_init.end()) {
            flag = true;
            Log::err << "Consumption of \'" << r.first << "\' does not have a resource pool.\n";
            break;
        }
    }
    if(!flag) {
        m_costs = cs;
    } else {
        Log::err << "Resource pool and consumption information error. Abosting chromosome initialization.\n";
        return;
    }

    for(unsigned int i = 0; i < Config::ga_population_size; i++) {
        m_population.push_back(GASChromosome(l));   /* Randomly initializes the new chromosome. */
    }

}

void GAScheduler::setAggregatedPayoff(unsigned int idx,
    const std::vector<sf::Vector2i>& /* cells */,
    const std::vector<float>& payoff,
    Aggregate type)
{
    float po;
    switch(type) {
        case Aggregate::MAX_VALUE:
        case Aggregate::MEAN_VALUE:
        case Aggregate::SUM_VALUE:
            po = 0.f;
            break;
        case Aggregate::MIN_VALUE:
            po = 1.f;
            break;
    }
    for(auto& p : payoff) {
        switch(type) {
            case Aggregate::MAX_VALUE:
                po = std::max(p, po);
                break;
            case Aggregate::MIN_VALUE:
                po = std::min(p, po);
                break;
            case Aggregate::MEAN_VALUE:
                po += p;
                break;
            case Aggregate::SUM_VALUE:
                po += p;
                break;
        }
    }
    if(type == Aggregate::MEAN_VALUE) {
        po /= payoff.size();
    }
    m_individual_info[idx].ag_payoff = po;
    // Log::dbg << "GAS Chromosome " << idx << " has a payoff of " << po << ".\n";
}

float GAScheduler::computeFitness(GASChromosome& c)
{
    float f = 0.f;                      /* Fitness. */
    float r = 0.f;                      /* Normalized and aggregated resource consumption. */
    std::map<std::string, float> rk;    /* Single resource capacity consumption (normalized). */

    /*  NOTE, `r` is computed as:
     *  R(k) = Σ consumption over t --> absolute accumulated consumption of resource `k`.
     *  `r` = (1/N) · Σ R(k)
     **/

    std::map<std::string, std::unique_ptr<Resource> > res_cpy;
    for(auto r : m_resources_init) {
        res_cpy[r.first] = std::unique_ptr<Resource>(r.second->clone());   /* Copies initial state. */
        rk[r.first] = 0.f;  /* Initialises normalized consumption counter.*/
    }
    int count_active_alleles = 0;
    for(unsigned int i = 0; i < c.alleles.size(); i++) {
        if(c.alleles.at(i)) {
            /* Allele is active: add payoff. */
            f += m_individual_info[i].ag_payoff;
            count_active_alleles++;
        }

        if(c.valid) {
            if(c.alleles.at(i)) {
                /* Allele is active: apply consumptions: */
                for(auto cost : m_costs) {
                    if(res_cpy[cost.first]->applyUntil(cost.second, m_individual_info[i].t_steps)) {
                        rk[cost.first] += cost.second * Config::time_step * m_individual_info[i].t_steps;
                    } else {
                        c.valid = false;
                        break;
                    }
                }
            } else {
                /* Allele is not active: simply step each resource. */
                for(auto cost : m_costs) {
                    res_cpy[cost.first]->applyUntil(0.f, m_individual_info[i].t_steps);
                }
            }
        }
    }
    if(c.valid) {
        /* There hasn't been resource violations: */
        for(auto rit : rk) {
            r += rit.second;
        }
        r /= rk.size();
        if(r == 0.f) {
            r = m_small_coeff;
        }
        f /= r;                                                     /* Minimize resource consumption. */
        f *= (count_active_alleles + 1) / (c.alleles.size() + 1);   /* Maximize number of activities. */
        if(count_active_alleles == 0) {
            c.valid = false;
        } else {
        }
    } else {
        f /= m_big_coeff;
    }
    c.fitness = f;
    return f;
}

GASChromosome GAScheduler::select(std::vector<GASChromosome>& mating_pool) const
{
    if(mating_pool.size() > 0) {
        GASChromosome retval(m_individual_info.size()); /* Initializes at random. */
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
            case GASSelectionOp::FITNESS_PROPORTIONATE_ROULETTE_WHEEL:
                {
                    float f_sum = 0.f;
                    std::sort(mating_pool.begin(), mating_pool.end(), std::less<GASChromosome>());
                    for(auto& ind : mating_pool) {
                        f_sum += ind.fitness;
                    }
                    float s = Random::getUf(0.f, f_sum);
                    for(auto it = mating_pool.begin(); it != mating_pool.end(); it++) {
                        s -= it->fitness;
                        if(s < 0.f) {
                            /* Threshold reached: */
                            retval = *it;
                            mating_pool.erase(it);
                            break;
                        }
                    }
                }
                break;
            case GASSelectionOp::STOCHASTIC_UNIVERSAL:
            case GASSelectionOp::ELITIST:
                Log::err << "Genetic Algorithm failure. Unimplemented operator (Parent selection).\n";
                throw std::runtime_error("Genetic Algorithm Scheduler failure: unimplemented selection operator case.");
            case GASSelectionOp::TRUNCATION:
            case GASSelectionOp::GENERATIONAL:
                Log::err << "Genetic Algorithm failure. Truncation and Generational selection operators ";
                Log::err << "are not suitable for parent selection.\n";
                throw std::runtime_error("Genetic Algorithm Scheduler failure, unimplemented parent selection operator case.");
        }
        return retval;
    } else {
        Log::err << "Can't select a parent if the mating pool is empty.\n";
        return GASChromosome(m_individual_info.size());
    }
}

GASChromosome GAScheduler::combine(std::vector<GASChromosome> parents, std::vector<GASChromosome> children)
{
    GASChromosome best_individual(m_individual_info.size());
    switch(Config::ga_environsel_op) {
        case GASSelectionOp::TRUNCATION:
        case GASSelectionOp::ELITIST:
            {
                auto pc = parents;
                pc.insert(pc.end(), children.begin(), children.end());
                std::sort(pc.begin(), pc.end(), std::greater<GASChromosome>());
                std::vector<GASChromosome> combination(pc.begin(), pc.begin() + Config::ga_population_size);
                m_population = std::move(combination);
                best_individual = m_population[0];
            }
            break;
        case GASSelectionOp::GENERATIONAL:
            m_population = std::move(children);
            std::sort(m_population.begin(), m_population.end(), std::greater<GASChromosome>());
            best_individual = m_population[0];
            break;
        case GASSelectionOp::TOURNAMENT:
        case GASSelectionOp::FITNESS_PROPORTIONATE_ROULETTE_WHEEL:
        case GASSelectionOp::STOCHASTIC_UNIVERSAL:
            Log::err << "Genetic Algorithm failure. Unimplemented operator (Parent-Children combination).\n";
            throw std::runtime_error("Genetic Algorithm Scheduler failure, unimplemented environment selection operator case.");
    }
    return best_individual;
}
