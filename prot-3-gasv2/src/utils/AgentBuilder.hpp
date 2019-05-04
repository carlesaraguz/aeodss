/***********************************************************************************************//**
 *  Generates an Agent from a YAML source or generates the YAML source and the Agent.
 *  @class      AgentBuilder
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-may-04
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef AGENT_BUILDER_HPP
#define AGENT_BUILDER_HPP

#include "prot.hpp"
#include <yaml-cpp/yaml.h>

#include "AgentMotion.hpp"

class AgentBuilder
{
public:
    AgentBuilder(void) = default;
    AgentBuilder(unsigned int id);                          /* Generates agent config. and the YAML file. */
    AgentBuilder(unsigned int id, std::string src_path);    /* Loads agent config. from an existing YAML file. */

    void generateAndStore(unsigned int id);
    void save(void);
    void load(unsigned int id, std::string src_path);

    std::string getAgentId(void) const { return m_agent_id; }
    OrbitalParams getOrbitalParams(void) const { return m_orbital_params; }
    double getMeanAnomalyInit(void) const { return m_mean_anomaly_init; }
    float getLinkRange(void) const { return m_link_range; }
    float getLinkDatarate(void) const { return m_link_datarate; }
    float getInstrumentEnergyRate(void) const { return m_instrument_energy_rate; }
    float getInstrumentStorageRate(void) const { return m_instrument_storage_rate; }
    float getInstrumentAperture(void) const { return m_instrument_aperture; }

private:
    std::string m_agent_id;
    OrbitalParams m_orbital_params;
    double m_mean_anomaly_init;
    float m_link_range;
    float m_link_datarate;
    float m_instrument_energy_rate;
    float m_instrument_storage_rate;
    float m_instrument_aperture;

    void randomize(void);
};

#endif /* AGENT_BUILDER_HPP */
