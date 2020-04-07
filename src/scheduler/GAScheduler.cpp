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
    , m_generation_timeout(0)
{
    m_population.reserve(Config::ga_population_size);
}

bool GAScheduler::iterate(unsigned int& g, GASChromosome best)
{
    bool do_continue = true;
    if(m_iteration_profile.size() == 0) {
        m_iteration_profile.push_back(std::make_pair(g, best.getFitness()));
    }
    if(g == 0) {
        m_generation_timeout = 0;
    }
    if(best == m_best) {
        m_generation_timeout++;
    } else if(best.getFitness() == m_best.getFitness()) {
        // Log::dbg << "GA Scheduler found a different solution but did not improve: " << best << ".\n";
        m_generation_timeout++;
    } else {
        m_best = best;
        m_generation_timeout = 0;
        /*  DEBUG:
         *  Log::dbg << "GA Scheduler found a better solution: " << best << ".\n";
         **/
        // Log::dbg << "GA Scheduler found a better solution: " << best << ".\n";
    }

    if(g >= Config::ga_generations) {
        do_continue = false;
        Log::dbg << "GA Scheduler reached the maximum amount of generations, stopping now.\n";
    } else if(m_best.isValid() && (m_generation_timeout >= Config::ga_timeout)) {
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

GASchedErr GAScheduler::schedule(std::vector<std::shared_ptr<Activity> >& adis, GAScheduler::Solution& result, bool debug)
{
    bool insert_bl = false;
    if(m_population.size() == 0) {
        Log::err << "Cannot start scheduling before population has been spawned.\n";
        throw std::runtime_error("GA Scheduler failed to start because population is not ready.");
    } else {
        if(m_previous_solutions.size() > 0) {
            if(Config::mode != SandboxMode::RANDOM) {
                /*  We may create 1 additional baseline solution which only has the previously scheduled tasks enabled.
                 **/
                GASChromosome cbaseline_prev(m_population[0].getChromosomeLength(), false); /* Initialises all alleles to false. */
                for(auto& ps : m_previous_solutions) {
                    for(unsigned int psaidx = ps.a_start; psaidx <= ps.a_end; psaidx++) {
                        cbaseline_prev.setAllele(psaidx, true);
                        insert_bl = true;
                    }
                }
                m_population.back() = cbaseline_prev;
            }
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
            m_max_cost[cost.first] += cost.second * (m_individual_info[act].t_end - m_individual_info[act].t_start);
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

    /*  We have previously inserted the 3rd type of baseline solution, and its fitness has just been computed.
     *  Check that it's valid:
     **/
    GASchedErr retval_ok = GASchedErr::FOUND_SOLUTION;
    if(insert_bl) {
        if(!m_population.back().isValid()) {
            Log::err << "Genetic Algorithm failure. The previous scheduling solution is invalid.\n";
            retval_ok = GASchedErr::PREVIOUS_SCHEDULE_INVALID;
        }
    }

    if(Config::mode == SandboxMode::RANDOM) {
        Log::dbg << "GA Scheduler will pick one solution at random, from the population of " << m_population.size() << ".\n";
        std::shuffle(std::begin(m_population), std::end(m_population), Random::getUniformEngine());

        for(auto solution = m_population.begin(); solution != m_population.end(); solution++) {
            if(solution->isValid()) {
                Log::dbg << "GA Scheduler found one valid random solution.\n";
                result = generateSolution(*solution, adis);
                return GASchedErr::FOUND_SOLUTION;
            }
        }
        Log::warn << "GA Scheduler could not find a solution.\n";
        result = GAScheduler::Solution();
        return GASchedErr::NO_SOLUTION;
    }

    if(m_init_individual.getChromosomeLength() <= 6) {
        GASChromosome best_exhaustive(m_population[0]);
        for(auto& sol : m_population) {
            if(sol.isValid() && sol > best_exhaustive) {
                best_exhaustive = sol;
            }
        }
        if(best_exhaustive.isValid()) {
            Log::dbg << "GA Scheduler completed after exhaustive search.\n";
            result = generateSolution(best_exhaustive, adis);
            return retval_ok;
        } else {
            Log::warn << "GA Scheduler completed after exhaustive search, but could not find a solution.\n";
            result = GAScheduler::Solution();
            return GASchedErr::NO_SOLUTION;
        }
    }

    /* Initialize control variables: */
    GASChromosome best(m_init_individual, true);  /* Randomly initializes, copying protected alleles. */
    unsigned int g = 0;
    m_iteration_profile.clear();
    Log::dbg << "GA Scheduler will start the evolutionary process now.\n";
    while(iterate(g, best)) {
        /* Repopulate in case we lost too many invalid options. */
        while(m_population.size() < Config::ga_population_size) {
            m_population.push_back(GASChromosome(m_init_individual, true));
        }

        std::vector<GASChromosome> children;
        std::vector<GASChromosome> parents = m_population;  /* Copy. */
        while(children.size() < m_population.size()) {
            GASChromosome parent1 = select(parents);
            GASChromosome parent2 = select(parents);
            GASChromosome child1(m_init_individual);
            GASChromosome child2(m_init_individual);
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
        Log::dbg << "GA Scheduler completed after " << g << " iterations.\n";
        if(debug) {
            Log::warn << "=== [GA SCHEDULER DEBUG MODE] === Showing results of fitness computation of solution:\n";
            computeFitness(best, true);
        }
        result = generateSolution(best, adis);
        return retval_ok;
    } else {
        Log::warn << "GA Scheduler completed after " << g << " iterations, but could not find a solution.\n";
        result = GAScheduler::Solution();
        return GASchedErr::NO_SOLUTION;
    }
}

GAScheduler::Solution GAScheduler::generateSolution(GASChromosome& c, std::vector<std::shared_ptr<Activity> >& adis)
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
    int dbg_prevsol_kept = 0;
    int dbg_prevsol_discarded = 0;
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
            for(unsigned int psa = ps.a_start; psa < ps.a_end + 1; psa++) {
                c.setAllele(psa, false);
            }
            dbg_prevsol_kept++;
            if(Config::verbosity) {
                Log::dbg << c << " -- " << ps.activity->getId() << " : Is kept [" << ps.a_start << "," << ps.a_end << "]\n";
            }
        } else {
            /* Does not modify the alleles and adds this activity to the discarded list. */
            adis.push_back(ps.activity);
            dbg_prevsol_discarded++;
            if(Config::verbosity) {
                Log::dbg << c << " -- " << ps.activity->getId() << " : Is discarded [" << ps.a_start << "," << ps.a_end << "]\n";
            }
        }
    }
    if(Config::verbosity) {
        Log::dbg << c << "\n";
    } else {
        Log::dbg << "GA Scheduler solution kept " << dbg_prevsol_kept << " and discarded " << dbg_prevsol_discarded << " previous activities.\n";
        Log::dbg << "GA Scheduler solution has " << c.getActivityCount() << " new activities.\n";
    }

    /* Now the chromosome only has alleles set for strictly NEW tasks: */
    double t0 = -1.0, t1 = -1.0;
    bool bflag = false;
    float bc = 0.f;
    int bc_count = 1;
    for(unsigned int i = 0; i < c.getChromosomeLength(); i++) {
        if(i > 0) {
            if(bflag && m_individual_info[i - 1].t_end < m_individual_info[i].t_start) {
                /* There's a gap that forces a finish. Record the previous activity: */
                bc /= (float)bc_count;
                retvec.push_back(std::make_tuple(t0, t1, bc));
                if(Config::verbosity) {
                    Log::dbg << " # New activity " << (retvec.size() - 1) << ": ["
                        << VirtualTime::toString(t0) << ", " << VirtualTime::toString(t1)
                        << "). B.conf: " << bc << "\n";
                }
                bflag = false;
            }
        }
        if(c.getAllele(i) && !bflag) {
            /* Start a new activity: */
            t0 = m_individual_info[i].t_start;
            t1 = m_individual_info[i].t_end;
            bc = m_individual_info[i].baseline_confidence;
            bc_count = 1;
            bflag = true;
        } else if(c.getAllele(i) && bflag) {
            /* Continue/extend a previous activity: */
            t1 = m_individual_info[i].t_end;
            bc += m_individual_info[i].baseline_confidence;
            bc_count++;
        }
        if(bflag && (!c.getAllele(i) || i == c.getChromosomeLength() - 1)) {
            /* Record the activity: */
            bc /= (float)bc_count;
            retvec.push_back(std::make_tuple(t0, t1, bc));
            if(Config::verbosity) {
                Log::dbg << " # New activity " << (retvec.size() - 1) << ": ["
                    << VirtualTime::toString(t0) << ", " << VirtualTime::toString(t1)
                    << "). B.conf: " << bc << "\n";
            }
            bflag = false;
        }
    }
    return retvec;
}

void GAScheduler::setChromosomeInfo(std::vector<double> t0s, std::vector<double> t1s, const std::map<std::string, double>& cs)
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
    if(t0s.size() != t1s.size()) {
        Log::err << "Chromosome start times and number of steps mismatch. Aborting setup.\n";
        return;
    } else {
        l = t0s.size();
        m_individual_info.reserve(l);
        for(unsigned int i = 0; i < l; i++) {
            GASInfo gas_info;
            gas_info.t_start = t0s[i];
            gas_info.t_end = t1s[i];
            if(gas_info.t_end - gas_info.t_start < (Config::time_step / 2.0)) {
                Log::err << "GA Scheduler is not capable of scheduling tasks the duration of which is less than 1/2 step. Aborting.\n";
                throw std::runtime_error("Genetic Algorithm Scheduler failure. Unable to solve for small activities.");
            }
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

    if(l <= 6 && Config::mode != SandboxMode::RANDOM) {
        /*  The length of the chromosomes is that small that we can skip the evolutionary process
         *  optimise the schedule by exhaustively creating every possible solution.
         *  Note that in random mode we let the population initialise normally and then we will pick
         *  one valid solution at random.
         **/
        for(unsigned int i = 0; i < std::pow(2, l); i++) {
            GASChromosome solution(l);
            std::string binary_chain = std::bitset<6>(i).to_string();
            for(unsigned int a = 0; a < l; a++) {
                solution.setAllele(a, (binary_chain[binary_chain.size() - 1 - a] == '1'));
            }
            m_population.push_back(solution);
        }
    } else {
        if(Config::mode != SandboxMode::RANDOM) {
            /*  If we're not in random mode, we insert three types of baseline options:
             *  - `l` solutions with a single activity enabled;
             *  - 1 solution where all activities are enabled; and
             *  - 1 solution where only previous activities are enabled (this is done within GAScheduler::schedule).
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
                if(m_population.size() < Config::ga_population_size) {
                    m_population.push_back(cbaseline);
                }
            }
            while(m_population.size() < Config::ga_population_size) {
                m_population.push_back(GASChromosome(l));   /* Randomly initializes the new chromosome. */
            }
        } else {
            /*  If we're in random mode, we insert randomized solutions with different
             *  allele-enabling probabilities.
             **/
            while(m_population.size() < Config::ga_population_size) {
                for(int p = 1; p <= 20; p++) {
                    float th = 0.05f * p;
                    if(m_population.size() < Config::ga_population_size) {
                        m_population.push_back(GASChromosome(l, true, th));
                    } else {
                        break;
                    }
                }
            }
        }
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
        default:
            Log::err << "Selected a wrong aggregate type for payoff.\n";
            std::exit(1);
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
            default:
                Log::err << "Selected a wrong aggregate type for payoff.\n";
                std::exit(1);
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
    /* Check for consistency: */
    if(a_start > m_individual_info.size() || a_end > m_individual_info.size()) {
        Log::err << "GA Scheduler: trying to configure a previous solution with wrong allele indices: [" << a_start << ", " << a_end << "]."
            << " Chromosome length is set to " << m_individual_info.size() << ".\n";
        return;
    } else if(a_end < a_start) {
        Log::err << "GA Scheduler: trying to configure a previous solution with wrong allele indices: " << a_start << " > " << a_end << ".\n";
        return;
    } else {
        /* Ensure these alleles do not exist in the list and/or do not overlap: */
        for(auto& ps : m_previous_solutions) {
            if(ps.a_start == a_start || ps.a_start == a_end || ps.a_end == a_start || ps.a_end == a_end) {
                Log::err << "GA Scheduler: trying to configure a previous solution with wrong allele indices: " << a_start << " or " << a_end
                    << " have already been recorded.\n";
                return;
            } else if((ps.a_start <= a_start && ps.a_end <= a_end && ps.a_end >= a_start) || (ps.a_start >= a_start && ps.a_end >= a_end && ps.a_start <= a_end)) {
                Log::err << "GA Scheduler: trying to configure a previous solution with wrong allele indices: [" << a_start << ", " << a_end
                    << "] overlap with [" << ps.a_start << ", " << ps.a_end << "].\n";
                return;
            }
        }
    }
    GASPrevSolution ga_ps = { a_start, a_end, aptr, 0.f };
    /* Compute lambda: */
    if(aptr->reportConfidence() >= Config::ga_confidence_th) {
        ga_ps.lambda = (aptr->reportConfidence() - Config::ga_confidence_th) * Config::ga_payoff_k;
        ga_ps.lambda /= (1.f - Config::ga_confidence_th);
    } else {
        ga_ps.lambda = 0.f;
    }
    m_previous_solutions.push_back(ga_ps);
    // Log::warn << "Configuring a previous solution [" << aptr->getAgentId() << ":" << aptr->getId()
    //     << "]. C: " << aptr->reportConfidence() << ", lambda = " << ga_ps.lambda << ". Allele start = " << a_start << ", end = " << a_end << "\n";
}


float GAScheduler::computeFitness(GASChromosome& c, bool verbose)
{
    float po = 0.f;                     /* Payoff. */
    // double r = 0.f;                  /* Normalized and aggregated resource consumption. */
    std::map<std::string, double> rk;   /* Single resource capacity consumption (normalized). */
    std::stringstream ss;

    /*  NOTE, `r` is computed as:
     *  R(k) = Σ consumption over t --> absolute accumulated consumption of resource `k`.
     *  R_norm(k) = R(k) / capacity.
     *  `r` = (1/N) · Σ R_norm(k)
     *  NOTE (2) : `r` is no longer used, but we keep it's implementation for reference.
     **/
    if(verbose) {
        Log::dbg << "GA Scheduler is computing fitness of chromosome: " << c << "\n";
        Log::dbg << std::fixed << std::setprecision(12) << "-- \n";
    }
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
            double dbg_capacity = res_cpy["energy"]->getCapacity();
            if(i > 0) {
                /* Check for gaps between alleles: */
                if(m_individual_info[i - 1].t_end < m_individual_info[i].t_start) {
                    /* There's a gap, these tasks are not contiguous. Step each resource accordingly. */
                    for(auto cost : m_costs) {
                        res_cpy[cost.first]->applyFor(0.f, (m_individual_info[i].t_start - m_individual_info[i - 1].t_end));
                    }
                    if(verbose) {
                        Log::dbg << "-- ( gap) -- (of --) -- #. T:{"
                            << VirtualTime::toString(m_individual_info[i - 1].t_end) << ", "
                            << VirtualTime::toString(m_individual_info[i].t_start) << "}. R:["
                            << dbg_capacity << ", " << res_cpy["energy"]->getCapacity() << "] --> " << (c.isValid() ? "valid" : "invalid") << "\n";
                    }
                    dbg_capacity = res_cpy["energy"]->getCapacity();
                }
            }
            if(c.getAllele(i)) {
                /* Allele is active: apply consumptions: */
                for(auto cost : m_costs) {
                    if(res_cpy[cost.first]->applyFor(cost.second, (m_individual_info[i].t_end - m_individual_info[i].t_start), verbose)) {
                        rk[cost.first] += cost.second * (m_individual_info[i].t_end - m_individual_info[i].t_start);
                    } else {
                        c.setValid(false);
                        break;
                    }
                }
            } else {
                /* Allele is not active: simply step each resource. */
                for(auto cost : m_costs) {
                    res_cpy[cost.first]->applyFor(0.f, (m_individual_info[i].t_end - m_individual_info[i].t_start));
                }
            }
            if(verbose) {
                Log::dbg << "-- Allele " << std::setw(2) << i << " (of " << c.getChromosomeLength() << ") -- " << c.getAllele(i) << ". T:{"
                    << VirtualTime::toString(m_individual_info[i].t_start) << ", "
                    << VirtualTime::toString(m_individual_info[i].t_end) << "}. R:["
                    << dbg_capacity << ", " << res_cpy["energy"]->getCapacity() << "] --> " << (c.isValid() ? "valid" : "invalid") << "\n";
            }
        } else {
            if(verbose) {
                Log::dbg << "-- Allele " << i << " (of " << c.getChromosomeLength() << ") -- Is no longer valid. Skipping the rest.\n";
            }
            break;
        }
    }
    if(count_active_alleles == 0) {
        c.setValid(false);
    }
    float fitness = 0.f;
    if(c.isValid()) {
        #if 0
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
        #endif

        /* Add additional payoff in case previous solutions have been maintained: *************** */
        for(auto& ps : m_previous_solutions) {
            /* Check wether this solution is kept: */
            bool sol_kept = true;
            float augmented_payoff = 0.f;
            for(unsigned int psa = ps.a_start; psa <= ps.a_end; psa++) {
                if(!c.getAllele(psa)) {
                    sol_kept = false;
                    if(verbose) {
                        Log::dbg << "-- One solution is not kept: [" << ps.a_start << ", " << ps.a_end << "].\n";
                    }
                    break;
                } else {
                    augmented_payoff += m_individual_info[psa].ag_payoff * ps.lambda;
                }
            }
            if(ps.activity->isConfimed() && !sol_kept) {
                Log::err << "Some chromosome did not keep a confirmed solution. This is unexpected\n";
                Log::err << c << "\n";
            }
            if(sol_kept) {
                /* The solution (i.e. activity) has been kept. Augment the chromosome payoff: */
                if(verbose) {
                    Log::dbg << "-- Previous is kept: [" << ps.a_start << ", " << ps.a_end << "]. PO = " << po;
                }
                po += augmented_payoff;
                if(verbose) {
                    Log::dbg << " --> " << po << "\n";
                }
            }
        }
        // po /= m_max_payoff; /* Normalise (not strictly necessary for this version). */
        fitness = po;       /* Before it was: [(po + r) / 2]. Could also be substituted by a Weighted Sum. */
    } else {
        fitness = 0.f;
    }
    c.setFitness(fitness);
    if(verbose) {
        Log::dbg << "-- Final fitness: " << fitness << "\n";
        Log::dbg << std::defaultfloat << "--\n";
    }
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

int GAScheduler::repairPool(std::vector<GASChromosome>& pool)
{
    int count_invalid = 0;
    for(auto ind = pool.begin(); ind != pool.end(); ) {
        if(!ind->isValid()) {
            ind = pool.erase(ind);
            count_invalid++;
        } else {
            ind++;
        }
    }
    return count_invalid;
}

GASChromosome GAScheduler::combine(std::vector<GASChromosome> parents, std::vector<GASChromosome>& children)
{
    GASChromosome best_individual(m_init_individual, true);
    if(parents.size() == 0 && children.size() == 0) {
        return best_individual;
    }
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
    Log::dbg << "GA Scheduler, debug info: ======================================================\n";
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
            cost += c.second * (ind_info.t_end - ind_info.t_start);
        }
        Log::dbg << "# " << count++
            << ": Tstart(" << VirtualTime::toString(ind_info.t_start)
            << "). Tend(" << VirtualTime::toString(ind_info.t_end)
            // << "). Steps(" << std::setw(5) << std::setfill('0') << ind_info.t_steps << std::setfill(' ')
            << "). AgPO(" << std::fixed << std::setprecision(1) << std::setw(6) << ind_info.ag_payoff
            << "). Cost(" << std::setprecision(4) << std::setw(8) <<  cost
            << "). Result: " << ind_info.ag_payoff / cost << std::defaultfloat << ".\n";
    }
    Log::dbg << "Previous solutions: ============================================================\n";
    for(auto ps : m_previous_solutions) {
        Log::dbg << "[" << ps.a_start << "," << ps.a_end << "], " << ps.lambda << " -> " << *ps.activity << "\n";
    }
    Log::dbg << "Initialisation individual: =====================================================\n";
    Log::dbg << m_init_individual << "\n";
    m_init_individual.printProtectedAlleles();
    Log::dbg << "{";
    unsigned int allele = 0;
    for(auto ps : m_previous_solutions) {
        for(; allele < ps.a_start; allele++) {
            Log::dbg << " ";
        }
        for(allele = ps.a_start; allele < ps.a_end + 1; allele++) {
            Log::dbg << "*";
        }
        allele = ps.a_end + 1;
    }
    for(; allele < m_init_individual.getChromosomeLength(); allele++) {
        Log::dbg << " ";
    }
    Log::dbg << "}\n";
}
