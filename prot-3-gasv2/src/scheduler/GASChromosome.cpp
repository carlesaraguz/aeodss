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

GASChromosome::GASChromosome(unsigned int sz)
    : alleles(sz)
    , valid(true)
    , fitness(0.f)
{
    for(unsigned int i = 0; i < sz; i++) {
        alleles[i] = (Random::getUf() > 0.5f);
    }
}

void GASChromosome::crossover(GASChromosome p1, GASChromosome p2, GASChromosome& c1, GASChromosome& c2)
{
    unsigned int l = p1.alleles.size();
    switch(Config::ga_crossover_op) {
        case GASCrossoverOp::SINGLE_POINT:
            {
                unsigned int xo_at = Random::getUi(0, l - 2);
                for(unsigned int i = 0; i < l; i++) {
                    if(i <= xo_at) {
                        /* Copy 1->1 and 2->2: */
                        c1.alleles[i] = p1.alleles[i];
                        c2.alleles[i] = p2.alleles[i];
                    } else {
                        /* Copy 1->2 and 2->1: */
                        c1.alleles[i] = p2.alleles[i];
                        c2.alleles[i] = p1.alleles[i];
                    }
                }
            }
            break;
        case GASCrossoverOp::MULIPLE_POINT:
            {
                std::vector<unsigned int> xo_points(l - 1);
                std::iota(xo_points.begin(), xo_points.end(), 0);  /* Generates 0, 1, 2, 3... (N-1). */
                int idx;
                if(Config::ga_crossover_points >= (l - 1)) {
                    /* Cross over all points: use xo_points as is. */
                } else {
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
                        c1.alleles[i] = p1.alleles[i];
                        c2.alleles[i] = p2.alleles[i];
                    } else {
                        /* Copy 1->2 and 2->1: */
                        c1.alleles[i] = p2.alleles[i];
                        c2.alleles[i] = p1.alleles[i];
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
                    c1.alleles[i] = p1.alleles[i];
                    c2.alleles[i] = p2.alleles[i];
                } else {
                    /* Copy 1->2 and 2->1. */
                    c1.alleles[i] = p2.alleles[i];
                    c2.alleles[i] = p1.alleles[i];
                }
            }
            break;
    }
}

void GASChromosome::mutate(void)
{
    for(unsigned int i = 0; i < alleles.size(); i++) {
        if(Random::getUf() <= Config::ga_mutation_rate) {
            alleles[i] = !alleles[i];   /* Bit flip: */
        }
    }
}

bool GASChromosome::operator>(const GASChromosome& rhs) const
{
    return fitness > rhs.fitness;
}

bool GASChromosome::operator<(const GASChromosome& rhs) const
{
    return fitness < rhs.fitness;
}

bool GASChromosome::operator>=(const GASChromosome& rhs) const
{
    return fitness >= rhs.fitness;
}

bool GASChromosome::operator<=(const GASChromosome& rhs) const
{
    return fitness <= rhs.fitness;
}

bool GASChromosome::operator==(const GASChromosome& rhs) const
{
    if(alleles.size() == rhs.alleles.size()) {
        bool bflag = true;
        for(std::size_t a = 0; a < alleles.size(); a++) {
            if(alleles[a] != rhs.alleles[a]) {
                bflag &= false;
                break;
            }
        }
        return bflag && (fitness == rhs.fitness);
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
    for(auto a : chr.alleles) {
        os << (int)a;
        count_active += (int)a;
    }
    os << " : " << count_active << " : " << std::fixed << std::setprecision(6) << std::setw(13) << chr.fitness << std::defaultfloat;
    if(!chr.valid) {
        os << "*";
    }
    os << "}";
    return os;
}
