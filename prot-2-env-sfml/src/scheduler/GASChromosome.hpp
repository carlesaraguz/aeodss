/***********************************************************************************************//**
 *  GA Scheduler chromosome.
 *  @class      GASChromosome
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-11
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
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
    GASChromosome(float t0 = 0.f, float t1 = 0.f, float maxd = 0.f);
    GASChromosome(GASChromosome p1, GASChromosome p2);
    GASChromosome(float t0, float t1, float maxd, std::vector<Intent> prev_res);
    GASChromosome(const GASChromosome& cpy);

    static void crossover(GASChromosome p1, GASChromosome p2, GASChromosome& c1, GASChromosome& c2);
    void mutate(void);
    void repair(void);

    float getStart(unsigned int taskid) const { return m_alleles[taskid].t_start; }
    float getDuration(unsigned int taskid) const { return m_alleles[taskid].t_duration; }
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
        float t_start;
        float t_duration;
    };

    std::vector<Allele> m_alleles;
    float m_fitness;
    float m_t0;
    float m_t1;
    float m_max_duration;

    static void copyAllele(const Allele& from, Allele& to);
};


#endif /* GAS_CHROMOSOME_HPP */
