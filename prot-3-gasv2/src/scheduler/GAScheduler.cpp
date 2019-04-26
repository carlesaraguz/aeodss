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

GAScheduler::GAScheduler(double t0, double t1, std::map<std::string, std::shared_ptr<const Resource> > res)
    : m_tstart(t0)
    , m_tend(t1)
    , m_resources_init(res)
    , m_best(0)
    , m_max_payoff(0.f)
    , m_init_individual(1)  /* arbitrary */
{
    m_population.reserve(Config::ga_population_size);
}

bool GAScheduler::iterate(unsigned int& g, GASChromosome best)
{
    bool do_continue = true;
    if(m_iteration_profile.size() == 0) {
        m_iteration_profile.push_back(std::make_pair(g, best.getFitness()));
    }
    if(best == m_best) {
        m_generation_timeout++;
    } else {
        m_best = best;
        m_generation_timeout = 0;
        /*  DEBUG:
         *  Log::dbg << "GA Scheduler found a better solution: " << best << ".\n";
         **/
    }

    if(g >= Config::ga_generations) {
        do_continue = false;
        Log::dbg << "GA Scheduler reached the maximum amount of generations, stopping now.\n";
    } else if(/* (g > Config::ga_generations / 3) && */ m_best.isValid() && (m_generation_timeout >= Config::ga_timeout)) {
        do_continue = false;
        Log::dbg << "GA Scheduler timed out after " << g << " generations, stopping now.\n";
        /*  NOTE:
         *  Previous version checked improvement rate and decided to complete the heuristic process
         *  by assessing that figure. The code is left commented below for future reference:
         *  ========================================================================================
         *    unsigned int g0 = m_iteration_profile[m_iteration_profile.size() - 1].first;
         *    float f0        = m_iteration_profile[m_iteration_profile.size() - 1].second;
         *    if(std::abs(f - f0) > Config::ga_min_improvement_rate) {
         *        float delta = (f - f0) / (g - g0);
         *        Log::dbg << "GA Scheduler, at g = " << g << ": f = " << f << ", Δ(abs) = " << std::abs(f - f0) << ", Δ(rel) = " << delta << "\n";
         *        m_iteration_profile.push_back(std::make_pair(g, f));
         *        if(delta < Config::ga_min_improvement_rate) {
         *            do_continue = false;
         *        }
         *    }
         **/
    } else {
        /*  NOTE: This part also belonged to the previous version (see comment above):
         *      m_iteration_profile[0].first  = g;
         *      m_iteration_profile[0].second = f;
         **/
        g++;
    }
    return do_continue;
}

GAScheduler::Solution GAScheduler::schedule(std::vector<std::shared_ptr<Activity> >& adis)
{
    if(m_population.size() == 0) {
        Log::err << "Cannot start scheduling before population has been spawned.\n";
        throw std::runtime_error("GA Scheduler failed to start because population is not ready.");
    } else {
        if(m_previous_solutions.size() > 0) {
            /*  We have encoded some previous solutions. Check whether we have to protect
             *  some alleles corresponding to confirmed activities.
             **/
            bool protect = false;
            std::vector<unsigned int> protected_alleles_idx;
            for(auto& ps : m_previous_solutions) {
                if(ps.activity->isConfimed()) {
                    protect = true;
                    for(unsigned int psaidx = ps.a_start; psaidx <= ps.a_end; psaidx++) {
                        protected_alleles_idx.push_back(psaidx);
                    }
                }
            }
            if(protect) {
                for(auto& ind : m_population) {
                    for(auto& ps : protected_alleles_idx) {
                        ind.setAllele(ps, true);    /* Forces this allele to true. */
                    }
                    ind.protect(protected_alleles_idx);
                }
            }
        }
    }

    if(m_individual_info.size() < 2) {
        Log::err << "GA Scheduler is configured with chromosomes of length " << m_individual_info.size() << ".\n";
    }

    /* Initialize max. resource consumptions and payoffs (needed to compute fitnesses): */
    for(auto& cost : m_costs) {
        if(m_max_cost.find(cost.first) == m_max_cost.end()) {
            m_max_cost[cost.first] = 0.f;
        }
        for(unsigned int act = 0; act < m_individual_info.size(); act++) {
            m_max_cost[cost.first] += cost.second * Config::time_step * m_individual_info[act].t_steps;
        }
        /*  DEBUG with:
         *  Log::dbg << "Max. \'" << cost.first << "\' cost: " << m_max_cost[cost.first] << ".\n";
         **/
    }
    for(unsigned int j = 0; j < m_individual_info.size(); j++) {
        m_max_payoff += m_individual_info[j].ag_payoff;
        for(auto ps : m_previous_solutions) {
            if(ps.a_start <= j && j <= ps.a_end) {
                m_max_payoff += m_individual_info[j].ag_payoff * ps.lambda;
            }
        }
    }
    m_init_individual = m_population[0];

    /* Initialize population fitness: */
    #pragma omp parallel for
    for(unsigned int i = 0; i < m_population.size(); i++) {
        computeFitness(m_population[i]);
    }
    /* DEBUG Baseline chromosomes:
     *  for(auto cbaseline = m_population.begin(); cbaseline != m_population.begin() + m_individual_info.size() + 1; cbaseline++) {
     *      Log::warn << "C. baseline: " << *cbaseline << "\n";
     *  }
     **/
    /* Initialize control variables: */
    GASChromosome best(m_init_individual);  /* Randomly initializes, copying protected alleles. */
    unsigned int g = 0;
    m_iteration_profile.clear();
    while(iterate(g, best)) {
        /* Repopulate in case we lost too many invalid options. */
        while(m_population.size() < Config::ga_population_size) {
            m_population.push_back(GASChromosome(m_init_individual));
        }

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

        if(g == 1) {
            repairPool(m_population);               /* Removes invalid parents. */
        }
        repairPool(children);                       /* Removes invalid children. */
        best = combine(m_population, children);     /* Environment selection: updates population. */

        /*  DEBUG:
         *  if(!prev_best_valid && best.isValid()) {
         *      Log::dbg << "GA Scheduler found a valid solution at generation " << std::setw(4) << g << ": " << best << ".\n";
         *  } else if(prev_best_valid && !best.isValid()) {
         *      Log::err << "GA Scheduler errored at " << std::setw(4) << g << ": best solution is no longer valid: " << best << ".\n";
         *  }
         *  prev_best_valid = best.isValid();
         **/
    }
    if(best.isValid()) {
        Log::dbg << "GA Scheduler completed after " << g << " iterations. Solution:\n";
        return generateSolution(best, adis);
    } else {
        Log::warn << "GA Scheduler completed after " << g << " iterations, but could not find a solution.\n";
        return GAScheduler::Solution();
    }
}

GAScheduler::Solution GAScheduler::generateSolution(GASChromosome c, std::vector<std::shared_ptr<Activity> >& adis)
{
    GAScheduler::Solution retvec;
    if(!c.isValid()) {
        return retvec;
    }
    adis.clear();

    /*  Detect whether previous solutions have been kept or not, and modify chromosome to prevent
     *  creating them again:
     **/
    c.protect({});  /* Unprotects all the alleles in this chromosome. */
    for(auto& ps : m_previous_solutions) {
        /* Check wether this solution is kept: */
        bool sol_kept = true;
        for(unsigned int psa = ps.a_start; psa <= ps.a_end; psa++) {
            if(!c.getAllele(psa)) {
                sol_kept = false;
                break;
            }
        }
        if(sol_kept) {
            /* Unset the alleles to prevent new activities to be created with this info. */
            for(unsigned int psa = ps.a_start; psa <= ps.a_end; psa++) {
                c.setAllele(psa, false);
            }
        } else {
            /* Does not modify the alleles and adds this activity to the discarded list. */
            adis.push_back(ps.activity);
        }
    }

    /* Now the chromosome only has alleles set for strictly NEW tasks: */
    double t0 = -1.0, t1 = -1.0;
    bool bflag = false;
    float bc = 0.f;
    int bc_count = 1;
    for(unsigned int i = 0; i < c.getChromosomeLength(); i++) {
        if(c.getAllele(i) && !bflag) {
            /* Start a new activity: */
            t0 = m_individual_info[i].t_start;
            t1 = m_individual_info[i].t_start + m_individual_info[i].t_steps * Config::time_step;
            bc = m_individual_info[i].baseline_confidence;
            bc_count = 1;
            bflag = true;
        } else if(c.getAllele(i) && bflag) {
            /* Continue/extend a previous activity: */
            t1 += m_individual_info[i].t_steps * Config::time_step;
            bc += m_individual_info[i].baseline_confidence;
            bc_count++;
        } else if(!c.getAllele(i) && bflag) {
            /* Record the activity: */
            bc /= (float)bc_count;
            retvec.push_back(std::make_tuple(t0, t1, bc));
            Log::dbg << " # Activity " << (retvec.size() - 1) << ": ["
                << VirtualTime::toString(t0) << ", " << VirtualTime::toString(t1)
                << "). B.conf: " << bc << "\n";
            bflag = false;
        }
    }
    if(bflag) {
        /* Record the last activity: */
        bc /= (float)bc_count;
        retvec.push_back(std::make_tuple(t0, t1, bc));
        Log::dbg << " # Activity " << (retvec.size() - 1) << ": ["
            << VirtualTime::toString(t0) << ", " << VirtualTime::toString(t1)
            << "). B.conf: " << bc << "\n";
    }
    return retvec;
}

void GAScheduler::setChromosomeInfo(std::vector<double> t0s, std::vector<int> s, const std::map<std::string, float>& cs)
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
            if(s[i] < 1) {
                Log::err << "GA Scheduler is not capable of scheduling tasks the duration of which is less than 1 step. Aborting.\n";
                throw std::runtime_error("Genetic Algorithm Scheduler failure. Unable to solve for small activities.");
            }
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

    /*  We always insert two types of baseline options:
     *  - `l` solutions with a single activity enabled; and
     *  - one solution where all activities are enabled.
     *  Note that this happens without checking protected alleles because it is supposed to happen
     *  before alleles are actually protected.
     **/
    for(unsigned int b = 0; b < l + 1; b++) {
        GASChromosome cbaseline(l);
        for(unsigned int a = 0; a < cbaseline.getChromosomeLength(); a++) {
            if(a == b && b < l) {
                cbaseline.setAllele(a, true);
            } else if(a != b && b < l) {
                cbaseline.setAllele(a, false);
            } else {
                cbaseline.setAllele(a, true);
            }
        }
        m_population.push_back(cbaseline);
    }

    while(m_population.size() < Config::ga_population_size) {
        m_population.push_back(GASChromosome(l));   /* Randomly initializes the new chromosome. */
    }
}

void GAScheduler::setAggregatedPayoff(unsigned int idx,
    const std::vector<sf::Vector2i>& /* cells */,
    const std::vector<float>& payoff,
    float baseline_confidence)
{
    float po;
    switch(Config::ga_payoff_aggregate) {
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
        switch(Config::ga_payoff_aggregate) {
            case Aggregate::MAX_VALUE:
                po = std::max(p, po);
                break;
            case Aggregate::MIN_VALUE:
                po = std::min(p, po);
                break;
            case Aggregate::MEAN_VALUE:
            case Aggregate::SUM_VALUE:
                po += p;
                break;
        }
    }
    if(Config::ga_payoff_aggregate == Aggregate::MEAN_VALUE) {
        po /= payoff.size();
    }
    m_individual_info[idx].ag_payoff = po;
    m_individual_info[idx].baseline_confidence = baseline_confidence;
}

void GAScheduler::setPreviousSolution(unsigned int a_start, unsigned int a_end, std::shared_ptr<Activity> aptr)
{
    GASPrevSolution ga_ps = { a_start, a_end, aptr, 0.f };
    /* Compute lambda: */
    if(aptr->reportConfidence() >= Config::ga_confidence_th) {
        ga_ps.lambda = (aptr->reportConfidence() - Config::ga_confidence_th) * Config::ga_payoff_k;
    }
    m_previous_solutions.push_back(ga_ps);
}


float GAScheduler::computeFitness(GASChromosome& c)
{
    float po = 0.f;                     /* Payoff. */
    float r = 0.f;                      /* Normalized and aggregated resource consumption. */
    std::map<std::string, float> rk;    /* Single resource capacity consumption (normalized). */

    /*  NOTE, `r` is computed as:
     *  R(k) = Σ consumption over t --> absolute accumulated consumption of resource `k`.
     *  R_norm(k) = R(k) / capacity.
     *  `r` = (1/N) · Σ R_norm(k)
     *  NOTE (2) : `r` is no longer used, but we keep it's implementation for reference.
     **/

    std::map<std::string, std::unique_ptr<Resource> > res_cpy;
    for(auto r : m_resources_init) {
        res_cpy[r.first] = std::unique_ptr<Resource>(r.second->clone());   /* Copies initial state. */
        rk[r.first] = 0.f;  /* Initialises normalized consumption counter.*/
    }
    int count_active_alleles = 0;
    for(unsigned int i = 0; i < c.getChromosomeLength(); i++) {
        if(c.getAllele(i)) {
            /* The allele is active, add its payoff: */
            po += m_individual_info[i].ag_payoff;
            count_active_alleles++;
        }

        if(c.isValid()) {
            if(c.getAllele(i)) {
                /* Allele is active: apply consumptions: */
                for(auto cost : m_costs) {
                    if(res_cpy[cost.first]->applyUntil(cost.second, m_individual_info[i].t_steps)) {
                        rk[cost.first] += cost.second * Config::time_step * m_individual_info[i].t_steps;
                    } else {
                        c.setValid(false);
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
    if(count_active_alleles == 0) {
        c.setValid(false);
    }
    float fitness = 0.f;
    if(c.isValid()) {
        /* There hasn't been resource violations, complete the calculation of the metric `r` */
        for(auto rit : rk) {
            r += rit.second / m_max_cost[rit.first];
        }
        r /= rk.size();
        r = 1.f - r;
        if(r == 0.f && count_active_alleles > 0) {
            r = m_small_coeff;
        }
        /**************************************************************************************** */

        /* Add additional payoff in case previous solutions have been maintained: *************** */
        for(auto& ps : m_previous_solutions) {
            /* Check wether this solution is kept: */
            bool sol_kept = true;
            float augmented_payoff = 0.f;
            for(unsigned int psa = ps.a_start; psa <= ps.a_end; psa++) {
                if(!c.getAllele(psa)) {
                    sol_kept = false;
                    break;
                } else {
                    augmented_payoff += m_individual_info[psa].ag_payoff * ps.lambda;
                }
            }
            if(sol_kept) {
                /* The solution (i.e. activity) has been kept. Augment the chromosome payoff: */
                po += augmented_payoff;
            }
        }
        po /= m_max_payoff; /* Normalise (not strictly necessary for this version). */
        fitness = po;       /* Before it was: [(po + r) / 2]. Could also be substituted by a Weighted Sum. */
    } else {
        fitness = 0.f;
    }
    c.setFitness(fitness);
    return fitness;
}

GASChromosome GAScheduler::select(std::vector<GASChromosome>& mating_pool) const
{
    if(mating_pool.size() > 0) {
        GASChromosome retval(m_individual_info.size()); /* Initializes at random. */
        switch(Config::ga_parentsel_op) {
            case GASSelectionOp::TOURNAMENT:
                {
                    bool bflag = false;
                    std::vector<GASChromosome>::iterator ind;
                    for(unsigned int k = 0; k < Config::ga_tournament_k; k++) {
                        ind = mating_pool.begin() + Random::getUi(0, mating_pool.size() - 1);
                        if(!bflag || *ind > retval) {
                            retval = *ind;
                            bflag = true;
                        }
                    }
                    mating_pool.erase(ind);
                }
                break;
            case GASSelectionOp::FITNESS_PROPORTIONATE_ROULETTE_WHEEL:
                {
                    float f_sum = 0.f;
                    std::sort(mating_pool.begin(), mating_pool.end(), std::less<GASChromosome>());
                    for(auto& ind : mating_pool) {
                        f_sum += ind.getFitness();
                    }
                    float s = Random::getUf(0.f, f_sum);
                    for(auto it = mating_pool.begin(); it != mating_pool.end(); it++) {
                        s -= it->getFitness();
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

void GAScheduler::repairPool(std::vector<GASChromosome>& pool)
{
    for(auto ind = pool.begin(); ind != pool.end(); ) {
        if(!ind->isValid()) {
            ind = pool.erase(ind);
        } else {
            ind++;
        }
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
                int elems = (pc.size() >= Config::ga_population_size ? Config::ga_population_size : pc.size());
                std::vector<GASChromosome> combination(pc.begin(), pc.begin() + elems);
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

void GAScheduler::debug(void) const
{
    Log::dbg << "GA Scheduler, debug info:\n";
    Log::dbg << "Costs: " << m_costs.size() << ".\n";

    if(m_individual_info.size() < 2) {
        Log::warn << "Activities: " << m_individual_info.size() << ".\n";
    } else {
        Log::dbg << "Activities: " << m_individual_info.size() << ".\n";
    }
    int count = 0;
    float cost;
    for(auto& ind_info : m_individual_info) {
        cost = 0.f;
        for(auto& c : m_costs) {
            cost += c.second * Config::time_step * ind_info.t_steps;
        }
        Log::dbg << "# " << count++
            << ": Tstart(" << VirtualTime::toString(ind_info.t_start)
            << "). Steps(" << std::setw(5) << std::setfill('0') << ind_info.t_steps << std::setfill(' ')
            << "). AgPO(" << std::fixed << std::setprecision(1) << std::setw(6) << ind_info.ag_payoff
            << "). Cost(" << std::setprecision(4) << std::setw(8) <<  cost
            << "). Result: " << ind_info.ag_payoff / cost << std::defaultfloat << ".\n";
    }
}
