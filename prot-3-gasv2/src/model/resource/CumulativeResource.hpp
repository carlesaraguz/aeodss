/***********************************************************************************************//**
 *  Resources that can be depleted and are cumulative.
 *  @class      CumulativeResource
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-07
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef CUMULATIVE_RESOURCE_HPP
#define CUMULATIVE_RESOURCE_HPP

#include "prot.hpp"
#include "Resource.hpp"
#include "Random.hpp"

class Activity;
class Agent;

class CumulativeResource : public Resource
{
public:
    CumulativeResource(Agent* aptr, std::string name, float max_a, float max_b, float c_init_a, float c_init_b);
    CumulativeResource(Agent* aptr, std::string name, float c, float c_init);
    CumulativeResource(Agent* aptr, std::string name, float c);

    float getCapacity(void) const override { return m_capacity; }
    float getMaxCapacity(void) const override { return m_max_capacity; }
    float getReservedCapacity(void) const override { return m_reserved_capacity; }
    void setMaxCapacity(float c) override;
    void setReservedCapacity(float c) override;
    void applyOnce(float c) override;
    bool applyUntil(float c, unsigned int steps);
    bool isFull(void) const override { return m_capacity == m_max_capacity; }
    bool isEmpty(void) const override { return m_capacity == 0.f; }
    bool tryApplyOnce(float c) const override;
    void addRate(float dc, Activity* ptr) override;
    void removeRate(Activity* ptr) override;
    void setName(std::string name) override { m_name = name; }
    std::string getName(void) const override { return m_name; }
    CumulativeResource* clone(void) const override { return new CumulativeResource(*this); }
    void showStatus(void) const;

    void step(void) override;

private:
    float m_capacity;
    float m_max_capacity;
    float m_reserved_capacity;
    Agent* m_agent;
    std::string m_name;
    float m_instantaneous;

    std::map<std::string, float> m_rates;
};

#endif /* CUMULATIVE_RESOURCE_HPP */
