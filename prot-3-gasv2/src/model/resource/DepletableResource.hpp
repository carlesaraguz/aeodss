/***********************************************************************************************//**
 *  Resources that can be depleted and are not cumulative.
 *  @class      DepletableResource
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-07
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef DEPLETABLE_RESOURCE_HPP
#define DEPLETABLE_RESOURCE_HPP

#include "prot.hpp"
#include "Resource.hpp"
#include "Random.hpp"

class Activity;
class Agent;

class DepletableResource : public Resource
{
public:
    DepletableResource(Agent* aptr, std::string name, double max_a, double max_b, double c_init_a, double c_init_b);
    DepletableResource(Agent* aptr, std::string name, double c, double c_init);
    DepletableResource(Agent* aptr, std::string name, double c);

    double getCapacity(void) const override { return m_capacity; }
    double getMaxCapacity(void) const override { return m_max_capacity; }
    double getReservedCapacity(void) const override { return m_reserved_capacity; }
    void setMaxCapacity(double c) override;
    void setReservedCapacity(double c) override;
    void applyOnce(double c) override;
    bool applyFor(double c, double t, bool verbose = false);
    bool isFull(void) const override { return m_capacity == m_max_capacity; }
    bool isEmpty(void) const override { return m_capacity == 0.f; }
    bool tryApplyOnce(double c) const override;
    void addRate(double dc, Activity* ptr) override;
    void removeRate(Activity* ptr) override;
    void setName(std::string name) override { m_name = name; }
    std::string getName(void) const override { return m_name; }
    DepletableResource* clone(void) const override { return new DepletableResource(*this); }
    void showStatus(void) const;

    void step(void) override;

private:
    double m_capacity;
    double m_max_capacity;
    double m_reserved_capacity;
    Agent* m_agent;
    std::string m_name;
    double m_instantaneous;

    std::map<std::string, double> m_rates;   /* Must only be positive. */
};

#endif /* DEPLETABLE_RESOURCE_HPP */
