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
    std::vector<bool> alleles;          /* The actual schedule. */
    float fitness;                      /* Fitness computed by scheduler. */
    bool valid;                         /* Whether resource violation occurs (=false) or not (=true). */

    static void crossover(GASChromosome p1, GASChromosome p2, GASChromosome& c1, GASChromosome& c2);
    void mutate(void);

    bool operator<(const GASChromosome& rhs) const;
    bool operator<=(const GASChromosome& rhs) const;
    bool operator>(const GASChromosome& rhs) const;
    bool operator>=(const GASChromosome& rhs) const;
    bool operator==(const GASChromosome& rhs) const;
    bool operator!=(const GASChromosome& rhs) const;

    friend std::ostream& operator<<(std::ostream& os, const GASChromosome& chr);
};


#endif /* GAS_CHROMOSOME_HPP */
