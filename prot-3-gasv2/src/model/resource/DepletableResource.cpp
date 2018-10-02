/***********************************************************************************************//**
 *  Resources that can be depleted and are not cumulative.
 *  @class      DepletableResource
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-07
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "DepletableResource.hpp"
#include "Agent.hpp"

CREATE_LOGGER(DepletableResource)

DepletableResource::DepletableResource(Agent* aptr, std::string name, float max_a, float max_b, float c_init_a, float c_init_b)
    : DepletableResource(aptr, name, Random::getUf(max_a, max_b), Random::getUf(c_init_a, c_init_b))
{ }

DepletableResource::DepletableResource(Agent* aptr, std::string name, float c)
    : DepletableResource(aptr, name, c, (c / 2.f))
{ }
DepletableResource::DepletableResource(Agent* aptr, std::string name, float c, float c_init)
    : m_max_capacity(c)
    , m_capacity(c_init)
    , m_name(name)
    , m_instantaneous(0.f)
    , m_agent(aptr)
{ }

void DepletableResource::setMaxCapacity(float c)
{
    if(c < m_capacity) {
        Log::err << "[Agent " << m_agent->getId() << ":" << m_name << "] Changing maximum capacity failed.\n";
        throw std::runtime_error("Resource capacity error.");
    }
    m_max_capacity = c;
}

void DepletableResource::applyOnce(float c)
{
    m_instantaneous += c;
}

void DepletableResource::addRate(float dc, Activity* ptr)
{
    m_rates[ptr] = dc;
}

void DepletableResource::removeRate(Activity* ptr)
{
    auto it = m_rates.find(ptr);
    if(it != m_rates.end()) {
        m_rates.erase(it);
    }
}

void DepletableResource::step(void)
{
    float acc = 0.f;
    for(auto& r : m_rates) {
        acc += r.second;
    }
    acc += m_instantaneous;
    if(acc > m_max_capacity) {
        Log::err << "[Agent " << m_agent->getId() << ":" << m_name << "] Trying to consume ["
            << m_capacity << "-]" << acc << " would result in negative capacity.\n";
        throw std::runtime_error("Resource capacity exceeded.");
    } else if(-acc > m_max_capacity) {
        m_capacity = m_max_capacity;
    } else if(m_max_capacity == acc) {
        Log::warn << "[Agent " << m_agent->getId() << ":" << m_name << "] Agent has depleted its resource completely (last consumption: "
            << acc << ").\n";
        m_capacity = m_max_capacity - acc;
    } else {
        m_capacity = m_max_capacity - acc;
    }
    m_instantaneous = 0.f;
}
