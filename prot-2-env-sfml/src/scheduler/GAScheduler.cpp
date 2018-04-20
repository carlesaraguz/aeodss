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
    : m_sched_win_start(0.f)
    , m_sched_win_end(0.f)
    , m_init_resource(0.f)
{
    m_population.reserve(Config::ga_population_size);
}

void GAScheduler::setRewards(std::vector<float> rewards)
{
    float t = m_sched_win_start;
    float dt = Config::time_step;
    m_rewards.clear();
    m_rewards.reserve(rewards.size());
    for(const auto& r : rewards) {
        m_rewards.push_back(GASReward(t, r));
        t += dt;
    }
}

void GAScheduler::setSchedulingWindow(float t0, float t1)
{
    if(t0 <= t1) {
        m_sched_win_start = t0;
        m_sched_win_end   = t1;
    } else {
        m_sched_win_start = t1;
        m_sched_win_end   = t0;
    }
}

void GAScheduler::initPopulation(std::vector<Intent> prev_res)
{
    m_population.clear();
    if(prev_res.size() == 0 || prev_res.size() > Config::max_tasks) {
        for(unsigned int i = 0; i < Config::ga_population_size; i++) {
            m_population.push_back(GASChromosome(m_sched_win_start, m_sched_win_end, Config::max_task_duration));
        }
    } else {
        for(unsigned int i = 0; i < Config::ga_population_size; i++) {
            m_population.push_back(GASChromosome(m_sched_win_start, m_sched_win_end, Config::max_task_duration, prev_res));
        }
    }
}

bool GAScheduler::stopGeneration(unsigned int gencount, float prev_fitmax, float fitmax, float /* fitmin */) const
{
    if(gencount < Config::ga_generations) {
        return false;
    } else if(gencount >= Config::ga_generations * 2) {
        return true;
    } else {
        if(fitmax == 0.f) {
            return false;
        }
        if((fitmax - prev_fitmax) / fitmax > 0.001f) {
            return true;
        } else {
            return false;
        }
        // float d = (fitmax - fitmin) / fitmax;
        // if(d > 0.01f) {
        //     return false;
        // } else {
        //     return true;
        // }
    }
}

std::vector<Intent> GAScheduler::schedule(void)
{
    GASChromosome best;
    float prev_fit_max = 0.f, fit_max = 0.f, fit_min = 0.f;
    float rmax = 0.f;

    unsigned int generation_count = 0;
    while(!stopGeneration(generation_count++, prev_fit_max, fit_max, fit_min)) {
        std::vector<GASChromosome> children;
        std::vector<GASChromosome> parents = m_population;  /* Copy. */
        while(children.size() < m_population.size()) {
            GASChromosome parent1 = selectParent(parents);
            GASChromosome parent2 = selectParent(parents);
            GASChromosome child1(0.f, 0.f, 0.f);
            GASChromosome child2(0.f, 0.f, 0.f);
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
        for(auto& c : children) {
            computeFitness(c, rmax);
        }
        best = combine(m_population, children);    /* Updates m_population. */
        prev_fit_max = fit_max;
        fit_max = best.getFitness();
        fit_min = m_population[m_population.size() - 1].getFitness();

        // std::cout << generation_count << "," << best.getFitness() << "," << m_population[m_population.size() - 1].getFitness() << "\n";
        if( (generation_count %   5 == 0 && generation_count < 1000 ) ||
            (generation_count %  50 == 0 && generation_count < 10000) ||
            (generation_count % 100 == 0)) {
            std::cout << "Fitness: "
                << std::fixed << std::setprecision(3) << std::setw(10) << fit_max << " / " << std::setw(10) << fit_min << ":: "
                << "(" << best.getActiveSlotCount() << ") Progress: ";
            std::cout << std::fixed << std::setprecision(1) << 50.f * (float)generation_count / Config::ga_generations << "%\r";
        }
    }
    std::cout << std::endl;

    auto retval = std::vector<Intent>();
    if(satisfiesConstraints(best)) {
        for(unsigned int iid = 0; iid < best.getTaskCount(); iid++) {
            if(best.isEnabled(iid)) {
                /* Constructs Intent only providing tstart and tend: */
                retval.emplace(retval.end(), best.getStart(iid), best.getStart(iid) + best.getDuration(iid));
            }
        }
    } else {
        std::cerr << "Best solution does not satisfy resource constraints:" << best;
    }
    return retval;
}

float GAScheduler::computeConsumption(const GASChromosome& ind) const
{
    float acc = 0.f;
    for(unsigned int task = 0; task < ind.getTaskCount(); task++) {
        if(ind.isEnabled(task)) {
            acc += ind.getDuration(task) * Config::capacity_consume;
        }
    }
    return acc;
}


void GAScheduler::computeFitness(GASChromosome& ind, float rnorm_factor)
{
    float modifier;
    float acc = 0.f, acc_resources = 0.f;
    for(unsigned int task = 0; task < ind.getTaskCount(); task++) {
        if(ind.isEnabled(task)) {
            std::cout << "/* message */1" << '\n';
            auto it0 = std::upper_bound(m_rewards.begin(), m_rewards.end(), GASReward(ind.getStart(task), 0.f));
            auto it1 = std::upper_bound(m_rewards.begin(), m_rewards.end(), GASReward(ind.getStart(task) + ind.getDuration(task), 0.f));
            if(it0 != m_rewards.end() && it1 != m_rewards.end()) {
                for(auto r = it0; r != it1; r++) {
                    if(r->value != 0) {
                        acc += r->value;
                    }
                }
            }
            std::cout << "/* message */2" << '\n';
            acc_resources += ind.getDuration(task) * Config::capacity_consume;
        }
    }
    acc_resources = 1.f - (acc_resources / rnorm_factor);
    if(satisfiesConstraints(ind)) {
        modifier = 1.f;
    } else {
        modifier = 1e-5f;
    }
    ind.setFitness(acc * acc_resources * modifier);
}

bool GAScheduler::satisfiesConstraints(GASChromosome ind, float* /* exceeded_res */) const
{
    std::vector<float> t0s;
    std::vector<float> t1s;
    for(unsigned int i = 0; i < ind.getTaskCount(); i++) {
        if(ind.isEnabled(i)) {
            t0s.push_back(ind.getStart(i));
            t1s.push_back(ind.getStart(i) + ind.getDuration(i));
        }
    }
    bool retval = true;
    float dr;
    float r = m_init_resource;
    float t = m_sched_win_start;
    for(std::size_t i = 0; i < t0s.size(); i++) {
        dr = Config::capacity_restore;
        if(r + dr * (t0s[i] - t) >= Config::max_capacity) {
            r  = Config::max_capacity;
        } else {
            r += dr * (t0s[i] - t);
        }
        dr = Config::capacity_restore - Config::capacity_consume;
        r += dr * (t1s[i] - t0s[i]);
        t  = t1s[i];
        if(r < 0.f) {
            return false;
        }
    }
    return retval;
}

GASChromosome GAScheduler::selectParent(std::vector<GASChromosome>& mating_pool) const
{
    GASChromosome retval(0.f, 0.f, 0.f); /* Initialises at random and with all fields = 0. */

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
            std::cout << "Genetic Algorithm failure. Unimplemented operator (P selection).\n";
            break;
        case GASSelectionOp::TRUNCATION:
        case GASSelectionOp::GENERATIONAL:
            std::cout << "Genetic Algorithm failure. Truncation and Generational selection operators ";
            std::cout << "are not suitable for parent selection.\n";
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
            std::cout << "Genetic Algorithm failure. Unimplemented operator (P-C combination).\n";
            break;
    }
    return best_individual;
}
