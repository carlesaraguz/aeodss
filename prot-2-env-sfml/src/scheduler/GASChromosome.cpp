/***********************************************************************************************//**
 *  GA Scheduler chromosome.
 *  @class      GASChromosome
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-11
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "GASChromosome.hpp"

GASChromosome::GASChromosome(unsigned int span, unsigned int maxd)
    : m_span(span)
    , m_max_duration(std::min(Config::max_task_duration, maxd))
    , m_fitness(0.f)
{
    /* Initialize chromosome randomly: */
    m_alleles.reserve(Config::max_tasks);
    for(unsigned int i = 0; i < Config::max_tasks; i++) {
        Allele a;
        a.en = (Random::getUf() > 0.5f);
        a.t_start = Random::getUi(0, m_span - 1);
        a.t_duration = Random::getUi(1, std::min(m_span - a.t_start, m_max_duration));
        m_alleles.push_back(a);
    }
}

GASChromosome::GASChromosome(GASChromosome p1, GASChromosome p2)
    : m_span(p1.m_span)
    , m_max_duration(p1.m_max_duration)
    , m_fitness(0.f)
{
    GASChromosome c1(p1.m_span, p1.m_max_duration);
    GASChromosome c2(p1.m_span, p1.m_max_duration);
    crossover(p1, p2, c1, c2);
    m_alleles.reserve(Config::max_tasks);
    for(unsigned int i = 0; i < Config::max_tasks; i++) {
        Allele a;
        a.en = c1.m_alleles[i].en;
        a.t_start = c1.m_alleles[i].t_start;
        a.t_duration = c1.m_alleles[i].t_duration;
        m_alleles.push_back(a);
    }
}

GASChromosome::GASChromosome(unsigned int span, unsigned int maxd, std::vector<Intent> /* prev_res */)
    : GASChromosome(span, maxd)
{
    std::cerr << "TODO! >> GASChromosome::GASChromosome(unsigned int span, unsigned int maxd, std::vector<Intent> prev_res)\n";
    std::exit(-1);

    /* TODO: */
    // std::vector<int> idxs(prev_res.size());
    // std::iota(idxs.begin(), idxs.end(), 0); /* Vector 0, 1, 2, 3... */
    // int idx;
    // while(idxs.size() > 0) {
    //     idx = idxs[Random::getUi(0, idxs.size() - 1)];
    //     m_alleles[idx].en = true;
    //     m_alleles[idx].t_start = prev_res.front().tstart;
    //     m_alleles[idx].t_duration = (prev_res.front().tend - prev_res.front().tstart);
    //     idxs.erase(std::find(idxs.begin(), idxs.end(), idx));
    // }
}

GASChromosome::GASChromosome(const GASChromosome& cpy)
    : m_span(cpy.m_span)
    , m_max_duration(cpy.m_max_duration)
    , m_fitness(cpy.m_fitness)
    , m_alleles(cpy.m_alleles)
{ }


void GASChromosome::crossover(GASChromosome p1, GASChromosome p2, GASChromosome& c1, GASChromosome& c2)
{
    c1.m_span = p1.m_span;
    c1.m_max_duration = p1.m_max_duration;
    c2.m_span = p2.m_span;
    c2.m_max_duration = p2.m_max_duration;
    switch(Config::ga_crossover_op) {
        case GASCrossoverOp::MULIPLE_POINT:
            {
                std::vector<unsigned int> xo_points(Config::max_tasks - 1);
                std::iota(xo_points.begin(), xo_points.end(), 0);
                int idx;
                if(Config::ga_crossover_points == (Config::max_tasks - 1)) {
                    /* Cross over all points: use xo_points as is. */
                } else {
                    /* Choose where to cross: remove points. */
                    for(unsigned int i = 0; i < ((Config::max_tasks - 1) - Config::ga_crossover_points); i++) {
                        idx = Random::getUi(0, xo_points.size() - 1);
                        xo_points.erase(xo_points.begin() + idx);
                    }
                }
                bool bflag = true;
                idx = 0;
                for(unsigned int i = 0; i < Config::max_tasks; i++) {
                    if(bflag) {
                        /* Copy alleles 1->1 and 2->2: */
                        c1.m_alleles[i] = p1.m_alleles[i];
                        c2.m_alleles[i] = p2.m_alleles[i];
                    } else {
                        /* Copy alleles 1->2 and 2->1: */
                        c1.m_alleles[i] = p2.m_alleles[i];
                        c2.m_alleles[i] = p1.m_alleles[i];
                    }
                    if(i == xo_points[idx]) {
                        bflag = !bflag;
                        if(xo_points.size() > 1) {
                            xo_points.erase(xo_points.begin());
                            idx++;
                        }
                    }
                }
            }
            break;

        case GASCrossoverOp::UNIFORM:
            for(unsigned int i = 0; i < Config::max_tasks; i++) {
                if(Random::getUf() > 0.5f) {
                    /* Copy alleles 1->1 and 2->2: */
                    c1.m_alleles[i] = p1.m_alleles[i];
                    c2.m_alleles[i] = p2.m_alleles[i];
                } else {
                    /* Copy alleles 1->2 and 2->1: */
                    c1.m_alleles[i] = p2.m_alleles[i];
                    c2.m_alleles[i] = p1.m_alleles[i];
                }
            }
            break;

        case GASCrossoverOp::SINGLE_POINT:
            int xo_at = Random::getUi(0, Config::max_tasks - 2);
            for(unsigned int i = 0; i < Config::max_tasks; i++) {
                if((int)i <= xo_at) {
                    /* Copy alleles 1->1 and 2->2: */
                    c1.m_alleles[i] = p1.m_alleles[i];
                    c2.m_alleles[i] = p2.m_alleles[i];
                } else {
                    /* Copy alleles 1->2 and 2->1: */
                    c1.m_alleles[i] = p2.m_alleles[i];
                    c2.m_alleles[i] = p1.m_alleles[i];
                }
            }
            break;
    }
}

void GASChromosome::mutate(void)
{
    int var;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dist(0.f, Config::ga_gaussian_mutation_std);
    for(auto& a : m_alleles) {
        if(Random::getUf() <= Config::ga_mutation_rate_enable) {
            /* Mutate enable value: */
            a.en = !a.en;
        }
        if(Random::getUf() <= Config::ga_mutation_rate_times) {
            /* Mutate start time: */
            var = std::round(Config::ga_gaussian_mutation_k1 * dist(gen));
            a.t_start += var;
            if(a.t_start < 0) {
                a.t_start = 0;
            } else if(a.t_start >= (int)(m_span - 1)) {
                a.t_start = m_span - 2;
            }
            if(a.t_start + a.t_duration > (int)m_span) {
                a.t_duration = m_span - a.t_start;
            }
        }
        if(Random::getUf() <= Config::ga_mutation_rate_times) {
            /* Mutate duration: */
            var = std::round(Config::ga_gaussian_mutation_k2 * dist(gen));
            a.t_duration += var;
            if(a.t_duration < 1) {
                a.t_duration = 1;
            } else if(a.t_start + a.t_duration > (int)m_span) {
                a.t_duration = m_span - a.t_start;
            }
        }

    }
}

void GASChromosome::repair(void)
{
    for(std::size_t i = 0; i < m_alleles.size(); i++) {
        if(m_alleles[i].en) {
            for(std::size_t j = 0; j < m_alleles.size(); j++) {
                if(i != j && m_alleles[j].en) {
                    /* Check overlap: */
                    int si = m_alleles[i].t_start;
                    int sj = m_alleles[j].t_start;
                    int ei = m_alleles[i].t_start + m_alleles[i].t_duration;
                    int ej = m_alleles[j].t_start + m_alleles[j].t_duration;

                    if((sj <= ei && sj >= si) || (si <= ej && si >= sj)) {
                        /* They overlap so we will merge them, and disable and re-initialize j: */
                        int t_end = std::max(
                            m_alleles[i].t_start + m_alleles[i].t_duration,
                            m_alleles[j].t_start + m_alleles[j].t_duration
                        );
                        m_alleles[i].t_start = std::min(m_alleles[i].t_start, m_alleles[j].t_start);
                        m_alleles[i].t_duration = t_end - m_alleles[i].t_start;

                        m_alleles[j].en = false;
                        m_alleles[j].t_start = Random::getUi(0, m_span - 1);
                        m_alleles[j].t_duration = Random::getUi(
                            1,
                            std::min(m_span - m_alleles[j].t_start, m_max_duration)
                        );
                    }
                }
            }
        }
    }
}

unsigned int GASChromosome::getActiveSlotCount(void) const
{
    int retval = 0;
    for(const auto& a : m_alleles) {
        if(a.en) {
            retval++;
        }
    }
    return retval;
}

bool GASChromosome::operator>(const GASChromosome& rhs) const
{
    return m_fitness > rhs.m_fitness;
}

bool GASChromosome::operator<(const GASChromosome& rhs) const
{
    return m_fitness < rhs.m_fitness;
}

bool GASChromosome::operator>=(const GASChromosome& rhs) const
{
    return m_fitness >= rhs.m_fitness;
}

bool GASChromosome::operator<=(const GASChromosome& rhs) const
{
    return m_fitness <= rhs.m_fitness;
}

bool GASChromosome::operator==(const GASChromosome& rhs) const
{
    return m_fitness == rhs.m_fitness;
}

bool GASChromosome::operator!=(const GASChromosome& rhs) const
{
    return m_fitness != rhs.m_fitness;
}


std::ostream& operator<<(std::ostream& os, const GASChromosome& chr)
{
    os << "Chromosome interval length: " << chr.m_span << ". Max. duration: " << chr.m_max_duration << " -- Fitness: " << chr.m_fitness << "\n";
    os << "   Alleles/tasks:\n";
    for(unsigned int i = 0; i < Config::max_tasks; i++) {
        os << "    - (" << i << ")  " << chr.m_alleles[i].en << " -- " << chr.m_alleles[i].t_start << ", " << chr.m_alleles[i].t_duration << "\n";
    }
    return os;
}
