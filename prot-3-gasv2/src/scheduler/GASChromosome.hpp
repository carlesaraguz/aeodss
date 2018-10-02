/***********************************************************************************************//**
 *  GA Scheduler chromosome.
 *  @class      GASChromosome
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-11
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef GAS_CHROMOSOME_HPP
#define GAS_CHROMOSOME_HPP

#include "prot.hpp"
#include "Random.hpp"
#include "Intent.hpp"
#include "GASOperators.hpp"

class GASChromosome
{
public:
    GASChromosome(unsigned int span = 0, unsigned int maxd = 0);
    GASChromosome(GASChromosome p1, GASChromosome p2);
    GASChromosome(unsigned int span, unsigned int maxd, std::vector<Intent> prev_res);
    GASChromosome(const GASChromosome& cpy);

    static void crossover(GASChromosome p1, GASChromosome p2, GASChromosome& c1, GASChromosome& c2);
    void mutate(void);
    void repair(void);

    int getStart(unsigned int taskid) const { return m_alleles[taskid].t_start; }
    int getDuration(unsigned int taskid) const { return m_alleles[taskid].t_duration; }
    bool isEnabled(unsigned int taskid) const { return m_alleles[taskid].en; }
    unsigned int getTaskCount(void) const { return m_alleles.size(); }
    unsigned int getActiveSlotCount(void) const;
    void setFitness(float fitness) { m_fitness = fitness; }
    float getFitness(void) const { return m_fitness; }

    bool operator>(const GASChromosome& rhs) const;
    bool operator<(const GASChromosome& rhs) const;
    bool operator>=(const GASChromosome& rhs) const;
    bool operator<=(const GASChromosome& rhs) const;
    bool operator==(const GASChromosome& rhs) const;
    bool operator!=(const GASChromosome& rhs) const;

    friend std::ostream& operator<<(std::ostream& os, const GASChromosome& chr);

private:
    struct Allele {
        bool en;
        int t_start;
        int t_duration;
    };

    std::vector<Allele> m_alleles;
    float m_fitness;
    unsigned int m_span;
    unsigned int m_max_duration;

    static void copyAllele(const Allele& from, Allele& to);
};


#endif /* GAS_CHROMOSOME_HPP */
