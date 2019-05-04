/***********************************************************************************************//**
 *  Generates an Agent from a YAML source or generates the YAML source and the Agent.
 *  @class      AgentBuilder
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-may-04
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "AgentBuilder.hpp"

CREATE_LOGGER(AgentBuilder)

AgentBuilder::AgentBuilder(unsigned int id)
    : m_agent_id("A" + std::to_string(id))
{
    generateAndStore(id);
}

void AgentBuilder::generateAndStore(unsigned int id)
{
    m_agent_id = "A" + std::to_string(id);
    randomize();
    save();
}

AgentBuilder::AgentBuilder(unsigned int id, std::string src_path)
{
    load(id, src_path);
}

void AgentBuilder::save(void)
{
    /* Store these values in a YAML file: */
    YAML::Emitter agent_conf;
    agent_conf.SetIndent(4);
    agent_conf << YAML::BeginMap;
    agent_conf << YAML::Key << m_agent_id;
    agent_conf << YAML::BeginMap;
    agent_conf << YAML::Key << "sma" << YAML::Value << m_orbital_params.sma;
    agent_conf << YAML::Key << "ecc" << YAML::Value << m_orbital_params.ecc;
    agent_conf << YAML::Key << "inc" << YAML::Value << m_orbital_params.inc;
    agent_conf << YAML::Key << "argp" << YAML::Value << m_orbital_params.argp;
    agent_conf << YAML::Key << "raan" << YAML::Value << m_orbital_params.raan;
    agent_conf << YAML::Key << "ma_init" << YAML::Value << m_mean_anomaly_init;
    agent_conf << YAML::Key << "link_range" << YAML::Value << m_link_range;
    agent_conf << YAML::Key << "link_datarate" << YAML::Value << m_link_datarate;
    agent_conf << YAML::Key << "inst_ap" << YAML::Value << m_instrument_aperture;
    agent_conf << YAML::Key << "inst_energy" << YAML::Value << m_instrument_energy_rate;
    agent_conf << YAML::Key << "inst_storage" << YAML::Value << m_instrument_storage_rate;
    agent_conf << YAML::EndMap;
    agent_conf << YAML::EndMap;

    std::ofstream yaml_file;
    yaml_file.open(Config::data_path + "system.yml", std::ios_base::app);
    if(yaml_file.is_open()) {
        yaml_file << agent_conf.c_str() << "\n";
        yaml_file.close();
    } else {
        Log::err << "Could not write agent " << m_agent_id << " configuration to \'" << Config::data_path + "system.yml" << "\'";
    }
}

void AgentBuilder::load(unsigned int id, std::string src_path)
{
    m_agent_id = "A" + std::to_string(id);
    try {
        YAML::Node agent_conf = YAML::LoadFile(src_path);
        YAML::Node an = agent_conf[m_agent_id];
        if(an.IsDefined()) {
            /* Recover values from YAML file: */
            m_orbital_params.sma      = an["sma"].as<double>();
            m_orbital_params.ecc      = an["ecc"].as<double>();
            m_orbital_params.inc      = an["inc"].as<double>();
            m_orbital_params.argp     = an["argp"].as<double>();
            m_orbital_params.raan     = an["raan"].as<double>();
            m_mean_anomaly_init       = an["ma_init"].as<double>();
            m_link_range              = an["link_range"].as<float>();
            m_link_datarate           = an["link_datarate"].as<float>();
            m_instrument_aperture     = an["inst_ap"].as<float>();
            m_instrument_energy_rate  = an["inst_energy"].as<float>();
            m_instrument_storage_rate = an["inst_storage"].as<float>();
        } else {
            Log::err << "Agent " << m_agent_id << " could not be found in \'" << src_path << "\'\n";
            Log::err << "Will generate random values from simulation config. file.\n";
            randomize();
        }
    } catch(const std::exception& e) {
        Log::err << "Error loading agent configuration from \'" << src_path << "\'.\n";
        Log::err << e.what() << "\n";
        Log::err << "Will generate random values from simulation config. file.\n";
        randomize();
    }
    save();
}

void AgentBuilder::randomize(void)
{
    m_orbital_params.sma    = (double)Random::getUf(Config::orbp_sma_max,  Config::orbp_sma_min);
    m_orbital_params.ecc    = (double)Random::getUf(Config::orbp_ecc_max,  0.f);
    m_orbital_params.inc    = (double)Random::getUf(Config::orbp_inc_max,  Config::orbp_inc_min);
    m_orbital_params.argp   = (double)Random::getUf(Config::orbp_argp_max, Config::orbp_argp_min);
    m_orbital_params.raan   = (double)Random::getUf(Config::orbp_raan_max, Config::orbp_raan_min);
    m_mean_anomaly_init     = MathUtils::degToRad(Random::getUf(Config::orbp_init_ma_max, Config::orbp_init_ma_min));
    m_link_range            = Random::getUf(Config::agent_range_min, Config::agent_range_max);
    m_link_datarate         = Random::getUf(Config::agent_datarate_min, Config::agent_datarate_max);
    m_instrument_aperture   = Random::getUf(Config::agent_aperture_min, Config::agent_aperture_max);
    m_instrument_energy_rate  = Random::getUf(Config::instrument_energy_min, Config::instrument_energy_max);
    m_instrument_storage_rate = Random::getUf(Config::instrument_storage_min, Config::instrument_storage_max);
}
