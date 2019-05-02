/***********************************************************************************************//**
 *  GA Scheduler chromosome.
 *  @class      GASChromosome
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-oct-08
 *  @version    0.2
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "GASChromosome.hpp"

CREATE_LOGGER(GASChromosome)

GASChromosome::GASChromosome(unsigned int sz)
    : m_alleles(sz)
    , m_protected_alleles(sz, false)
    , m_valid(true)
    , m_fitness(0.f)
{
    for(unsigned int i = 0; i < sz; i++) {
        m_alleles[i] = (Random::getUf() > 0.5f);
    }
}

GASChromosome::GASChromosome(const GASChromosome& other, bool randomize)
    : m_alleles(other.m_alleles)
    , m_protected_alleles(other.m_protected_alleles)
    , m_valid(other.m_valid)
    , m_fitness(other.m_fitness)
{
    if(randomize) {
        for(unsigned int i = 0; i < other.getChromosomeLength(); i++) {
            if(m_protected_alleles[i]) {
                m_alleles[i] = other.getAllele(i);
            } else {
                m_alleles[i] = (Random::getUf() > 0.5f);
            }
        }
    }
}

void GASChromosome::crossover(GASChromosome p1, GASChromosome p2, GASChromosome& c1, GASChromosome& c2)
{
    if(p1.m_alleles.size() != p2.m_alleles.size() || p2.m_alleles.size() != c1.m_alleles.size() || c1.m_alleles.size() != c2.m_alleles.size()) {
        Log::err << "Error on chromosome crossover operator: size mismatch.\n";
        Log::err << "Parent sizes are " << p1.m_alleles.size() << " and " << p2.m_alleles.size()
            << ". Children sizes are " << c1.m_alleles.size() << " and " << c2.m_alleles.size() << ".\n";
        throw std::runtime_error("Error on chromosome crossover operator: size mismatch.");
    }
    unsigned int l = p1.m_alleles.size();
    /* Fallback in case of error: */
    if(l <= 1) {
        if(Random::getUf() > 0.5f) {
            c1.m_alleles = p1.m_alleles;
            c2.m_alleles = p2.m_alleles;
        } else {
            c1.m_alleles = p2.m_alleles;
            c2.m_alleles = p1.m_alleles;
        }
        return;
    }
    /* Check that both parents share the same protected alleles, with the same value: */
    for(unsigned int pa = 0; pa < l; pa++) {
        if(p1.m_protected_alleles[pa] == p2.m_protected_alleles[pa]) {
            if(p1.m_protected_alleles[pa] && (p1.m_alleles[pa] != p2.m_alleles[pa])) {
                Log::err << "Two chromosomes have different values in protected alleles.\n";
                throw std::runtime_error("Unable to crossover: protected alleles mismatch.");
            }
        } else {
            Log::err << "Two chromosomes don't share the same protected alleles.\n";
            throw std::runtime_error("Unable to crossover: protected alleles mismatch.");
        }
    }
    /* Continue with crossover: */
    switch(Config::ga_crossover_op) {
        case GASCrossoverOp::SINGLE_POINT:
            {
                unsigned int xo_at = Random::getUi(0, l - 2);
                for(unsigned int i = 0; i < l; i++) {
                    if(i <= xo_at) {
                        /* Copy 1->1 and 2->2: */
                        c1.m_alleles[i] = p1.m_alleles[i];
                        c2.m_alleles[i] = p2.m_alleles[i];
                    } else {
                        /* Copy 1->2 and 2->1: */
                        c1.m_alleles[i] = p2.m_alleles[i];
                        c2.m_alleles[i] = p1.m_alleles[i];
                    }
                }
            }
            break;
        case GASCrossoverOp::MULIPLE_POINT:
            {
                std::vector<unsigned int> xo_points(l - 1);
                std::iota(xo_points.begin(), xo_points.end(), 0);  /* Generates 0, 1, 2, 3... (N-1). */
                int idx;
                if(Config::ga_crossover_points < (l - 1)) {
                    /* Choose where to cross by removing some XO points. */
                    for(unsigned int i = 0; i < ((l - 1) - Config::ga_crossover_points); i++) {
                        idx = Random::getUi(0, xo_points.size() - 1);
                        xo_points.erase(xo_points.begin() + idx);
                    }
                }
                bool bflag = true;
                idx = 0;
                for(unsigned int i = 0; i < l; i++) {
                    if(bflag) {
                        /* Copy 1->1 and 2->2: */
                        c1.m_alleles[i] = p1.m_alleles[i];
                        c2.m_alleles[i] = p2.m_alleles[i];
                    } else {
                        /* Copy 1->2 and 2->1: */
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
            for(unsigned int i = 0; i < l; i++) {
                if(Random::getUf() > 0.5f) {
                    /* Copy 1->1 and 2->2. */
                    c1.m_alleles[i] = p1.m_alleles[i];
                    c2.m_alleles[i] = p2.m_alleles[i];
                } else {
                    /* Copy 1->2 and 2->1. */
                    c1.m_alleles[i] = p2.m_alleles[i];
                    c2.m_alleles[i] = p1.m_alleles[i];
                }
            }
            break;
    }
}

void GASChromosome::mutate(void)
{
    for(unsigned int i = 0; i < m_alleles.size(); i++) {
        if(!m_protected_alleles[i]) {
            if(Random::getUf() <= Config::ga_mutation_rate) {
                m_alleles[i] = !m_alleles[i];   /* Bit flip: */
            }
        }
    }
}
void GASChromosome::protect(std::vector<unsigned int> alleles_idxs)
{
    std::vector<bool> new_vec(m_protected_alleles.size(), false);
    m_protected_alleles.swap(new_vec);    /* Reset. */

    for(auto& a : alleles_idxs) {
        if(a < m_protected_alleles.size()) {
            m_protected_alleles[a] = true;  /* This allele can no longer be changed. */
        } else {
            Log::warn << "Trying to protect an allele whose index is out of bounds.\n";
        }
    }
}

void GASChromosome::setAllele(unsigned int a, bool v)
{
    if(!m_protected_alleles[a]) {
        m_alleles[a] = v;
    } else {
        Log::err << "Trying to set an allele value that has been protected.\n";
    }
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
    if(m_alleles.size() == rhs.m_alleles.size()) {
        bool bflag = true;
        for(std::size_t a = 0; a < m_alleles.size(); a++) {
            if(m_alleles[a] != rhs.m_alleles[a]) {
                bflag &= false;
                break;
            }
        }
        return bflag && (m_fitness == rhs.m_fitness);
    } else {
        return false;
    }
}

bool GASChromosome::operator!=(const GASChromosome& rhs) const
{
    return !(*this == rhs);
}

std::ostream& operator<<(std::ostream& os, const GASChromosome& chr)
{
    int count_active = 0;
    os << "{";
    for(const auto& a : chr.m_alleles) {
        os << (int)a;
        count_active += (int)a;
    }
    os << " : " << count_active << " : " << std::fixed << std::setprecision(4) << std::setw(6) << chr.m_fitness << std::defaultfloat;
    if(!chr.m_valid) {
        os << "*";
    }
    os << "}";
    return os;
}

void GASChromosome::printProtectedAlleles(void) const
{
    /* DEBUG purposes: */
    Log::dbg << "{";
    for(const auto& pa : m_protected_alleles) {
        if(pa) {
            Log::dbg << "#";
        } else {
            Log::dbg << "-";
        }
    }
    Log::dbg << "}\n";
}
