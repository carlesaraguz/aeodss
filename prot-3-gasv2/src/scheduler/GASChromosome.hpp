/***********************************************************************************************//**
 *  GA Scheduler chromosome.
 *  @class      GASChromosome
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-oct-08
 *  @version    0.2
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef GAS_CHROMOSOME_HPP
#define GAS_CHROMOSOME_HPP

#include "prot.hpp"
#include "Random.hpp"

class GASChromosome
{
public:
    GASChromosome(unsigned int sz);
    GASChromosome(const GASChromosome& other);

    static void crossover(GASChromosome p1, GASChromosome p2, GASChromosome& c1, GASChromosome& c2);
    void mutate(void);
    void protect(std::vector<unsigned int> alleles_idxs);

    void setAllele(unsigned int a, bool v);
    unsigned int getChromosomeLength(void) const { return m_alleles.size(); }
    bool isProtected(unsigned int a) const { return m_protected_alleles[a]; }
    bool getAllele(unsigned int a) const { return m_alleles[a]; }
    float getFitness(void) const { return m_fitness; }
    void setFitness(float f) { m_fitness = f; }
    bool isValid(void) const { return m_valid; }
    void setValid(bool v = true) { m_valid = v; }

    bool operator<(const GASChromosome& rhs) const;
    bool operator<=(const GASChromosome& rhs) const;
    bool operator>(const GASChromosome& rhs) const;
    bool operator>=(const GASChromosome& rhs) const;
    bool operator==(const GASChromosome& rhs) const;
    bool operator!=(const GASChromosome& rhs) const;

    friend std::ostream& operator<<(std::ostream& os, const GASChromosome& chr);

private:
    std::vector<bool> m_alleles;          /* The actual schedule. */
    float m_fitness;                      /* Fitness computed by scheduler. */
    bool m_valid;                         /* Whether resource violation occurs (=false) or not (=true). */
    std::vector<bool> m_protected_alleles;
};


#endif /* GAS_CHROMOSOME_HPP */
