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
    , m_reserved_capacity(0.f)
{ }

void DepletableResource::setMaxCapacity(float c)
{
    if(c < m_capacity) {
        Log::err << "[Agent " << m_agent->getId() << ":" << m_name << "] Changing maximum capacity failed.\n";
        throw std::runtime_error("Resource capacity error.");
    }
    m_max_capacity = c;
}

bool DepletableResource::tryApplyOnce(float c) const
{
    float acc = 0.f;
    for(auto& r : m_rates) {
        acc += r.second;
    }
    acc += m_instantaneous + c;
    return acc <= m_max_capacity - m_reserved_capacity;
}

void DepletableResource::applyOnce(float c)
{
    m_instantaneous += c;
}

bool DepletableResource::applyUntil(float c, unsigned int steps)
{
    if(steps == 0) {
        return true;
    }

    float acc = 0.f;
    for(auto& r : m_rates) {
        acc += r.second;
    }
    acc += c;
    if(m_max_capacity - acc - m_instantaneous >= m_reserved_capacity) {
        m_capacity = m_max_capacity - acc - m_instantaneous;
        return true;
    } else {
        m_capacity = m_reserved_capacity;
        return false;
    }
}

void DepletableResource::addRate(float dc, Activity* ptr)
{
    std::string rate_id;
    if(ptr == nullptr) {
        rate_id = "undefined";
    } else {
        rate_id = ptr->getAgentId() + ":" + std::to_string(ptr->getId());
    }
    if(dc > 0.f) {
        m_rates[rate_id] = dc;
    } else {
        Log::warn << "Can't inflict a negative consumption rate (" << rate_id << ") for the depletable resource \'" << m_name << "\'\n";
    }
}

void DepletableResource::removeRate(Activity* ptr)
{
    std::string rate_id;
    if(ptr == nullptr) {
        rate_id = "undefined";
    } else {
        rate_id = ptr->getAgentId() + ":" + std::to_string(ptr->getId());
    }
    auto it = m_rates.find(rate_id);
    if(it != m_rates.end()) {
        m_rates.erase(it);
    } else {
        Log::err << "Could not remove resource consumption rate for activity " << rate_id << " and resource \'" << m_name << "\'.\n";
    }
}

void DepletableResource::step(void)
{
    float acc = 0.f;
    for(auto& r : m_rates) {
        acc += r.second;    /* These "rates" will only be positive for depletable resources. */
    }
    acc += m_instantaneous;
    if(acc > m_max_capacity - m_reserved_capacity) {
        Log::err << "[Agent " << m_agent->getId() << ":" << m_name << "] Trying to consume ["
            << (m_max_capacity - m_reserved_capacity) << "-]" << acc << " would result in negative capacity.\n";
        throw std::runtime_error("Resource capacity exceeded.");
    } else if(m_max_capacity - m_reserved_capacity == acc) {
        Log::warn << "[Agent " << m_agent->getId() << ":" << m_name << "] Agent has depleted its resource completely (last consumption: "
            << acc << ").\n";
        m_capacity = m_max_capacity - acc;
    } else {
        m_capacity = m_max_capacity - acc;
    }
    m_instantaneous = 0.f;
}

void DepletableResource::showStatus(void) const
{
    Log::dbg << "Resource status [" << m_name << "]: capacity is " << m_capacity << "/" << m_max_capacity
        << " (" << std::fixed << std::setprecision(0) << (100.f * m_capacity / m_max_capacity) << "%). Active rates: " << m_rates.size() << ".\n";
    for(auto& r : m_rates) {
        Log::dbg << " # " << r.first << " -> " << std::defaultfloat << r.second << ".\n";
    }
    Log::dbg << std::defaultfloat;
}
