/***********************************************************************************************//**
 *  Program configuration values.
 *  @class      Config
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-nov-23
 *  @version    0.3
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Config.hpp"
#include "ColorGradient.hpp"
#include "Random.hpp"

CREATE_LOGGER(Config)

#define CONF_VERSION  1

/*  NOTE:
 *  Some of the following are variables might be ODR-used somewhere and, therefore, could need a
 *  definition in the source. To prevent unexpected behaviour in the compilation process, all have
 *  been explicitly defined below (even variables that could be initialized in headers).
 *  For more information, please refer to the pages below:
 *    - Static const integral variables:    https://stackoverflow.com/a/45719501/1876268
 *    - One-Definition-Rule definition:     https://en.cppreference.com/w/cpp/language/definition
 **/

/* General constants: */
const double Config::pi = std::acos(-1.0);

/* Configuration constants: */
unsigned int    Config::win_width  = 1800;
unsigned int    Config::win_height =  900;
unsigned int    Config::world_width  = 1800;
unsigned int    Config::world_height =  900;
unsigned int    Config::model_unity_size = 10;
unsigned int    Config::agent_size = 14;
unsigned int    Config::n_agents = 1;
double          Config::start_epoch = 2451545.0;
double          Config::duration = 30.0;
double          Config::time_step = 10.0 / 86400.0;
float           Config::max_revisit_time = 0.7f;
float           Config::target_revisit_time = 0.2f;
float           Config::min_payoff = 1e-3f;
float           Config::max_payoff = 1.f;

/* Earth WGS84 parameters: */
const double    Config::earth_radius  = 6371000.0;
const double    Config::earth_wgs84_a = 6378137.0;
const double    Config::earth_wgs84_b = 6356752.314245;
const double    Config::earth_wgs84_e = 0.08181919;
const double    Config::earth_mu = 3.986004419e14;

/* Agent parametrization: */
float           Config::agent_aperture_min = 60.f;
float           Config::agent_aperture_max = 120.f;
float           Config::agent_range_min = 50.f;
float           Config::agent_range_max = 90.f;
float           Config::agent_datarate_min = 100.f;
float           Config::agent_datarate_max = 200.f;
bool            Config::link_allow_during_capture = false;
float           Config::agent_speed =  4.f;
unsigned int    Config::agent_planning_window = 1080; /* Steps (540 ~= 1 orbit). */
float           Config::activity_size = 1000.f;
AgentMotionType Config::motion_model = AgentMotionType::ORBITAL;
TimeValueType   Config::time_type;

/* Orbital motion random paramters: */
float           Config::orbp_sma_min = Config::earth_wgs84_a + 500e3f;
float           Config::orbp_sma_max = Config::earth_wgs84_a + 900e3f;
float           Config::orbp_ecc_max = 0.f;
float           Config::orbp_inc_min = 0.f;
float           Config::orbp_inc_max = 90.f;
float           Config::orbp_argp_min = 0.f;
float           Config::orbp_argp_max = 360.f;
float           Config::orbp_raan_min = 0.f;
float           Config::orbp_raan_max = 360.f;
float           Config::orbp_init_ma_max = 0.f;
float           Config::orbp_init_ma_min = 360.f;

/* Resource consumptions and capacities: */
/* -- Energy: */
float Config::agent_energy_generation_rate = -764.f;
float Config::instrument_energy_min = 900.f;
float Config::instrument_energy_max = 1200.f;
float Config::link_tx_energy_rate = 0.5f;
float Config::link_rx_energy_rate = 0.05f;
/* -- Storage: */
float Config::instrument_storage_min = 0.01f;
float Config::instrument_storage_max = 0.01f;

/* Format and colors: */
unsigned int    Config::fnt_size = 24;
sf::Font        Config::fnt_monospace;
sf::Font        Config::fnt_normal;
sf::Font        Config::fnt_awesome;
sf::Color       Config::color_orange = {255, 153, 102};
sf::Color       Config::color_dark_green = {35, 94, 92};
sf::Color       Config::color_link_los = {100, 100, 100};
sf::Color       Config::color_link_connected = {255, 153, 102, 127};
sf::Color       Config::color_link_sending = {151, 45, 168};
ColorGradient   Config::color_gradient_rgb;
ColorGradient   Config::color_gradient_rbg;
ColorGradient   Config::color_gradient_krbg;
ColorGradient   Config::color_gradient_rainbow;
ColorGradient   Config::color_gradient_1;
ColorGradient   Config::color_gradient_blue;

/* Scheduling hard constraints: */
unsigned int    Config::max_tasks = 25;
double          Config::max_task_duration = 7.0 * 60.0 / 86400.0;    /* 7 min. in JD */

/* Genetic Algorithm configuration: */
unsigned int    Config::ga_generations = 10000;
unsigned int    Config::ga_timeout = 1000;
float           Config::ga_min_improvement_rate = 0.01f;
unsigned int    Config::ga_population_size = 500;
unsigned int    Config::ga_crossover_points = 5;
unsigned int    Config::ga_tournament_k = 2;
float           Config::ga_mutation_rate = 0.2f;
GASCrossoverOp  Config::ga_crossover_op = GASCrossoverOp::MULIPLE_POINT;
GASSelectionOp  Config::ga_parentsel_op = GASSelectionOp::TOURNAMENT;
GASSelectionOp  Config::ga_environsel_op = GASSelectionOp::ELITIST;

/* Global values: */
std::string Config::root_path;
std::string Config::data_path;

void Config::loadCmdArgs(int argc, char** argv)
{
    std::string opt, opt_val;
    for(int cmd_idx = 1; cmd_idx < argc; cmd_idx++) {
        opt = argv[cmd_idx];
        if(opt == "-f" && (cmd_idx + 1) < argc) {
            opt_val = argv[cmd_idx + 1];
            try {
                YAML::Node conf = YAML::LoadFile(root_path + "conf/" + opt_val);
                Log::dbg << "Loading configuration from \'PROJECT_ROOT/" << ("conf/" + opt_val) << "\'.\n";
                if(conf["version"].IsDefined()) {
                    unsigned int conf_ver = conf["version"].as<unsigned int>();
                    if(conf_ver < CONF_VERSION) {
                        Log::warn << "Configuration file version " << conf_ver << " is older than the implementation.\n";
                    } else if(conf_ver > CONF_VERSION) {
                        Log::err << "Unexpected config. file version " << conf_ver
                            << ". Implementation only supports versions <= " << CONF_VERSION << ".\n";
                        throw std::runtime_error("Unsupported configuration file. Wrong version specs.");
                    }
                }
                for(auto node_it : conf) {
                    if(node_it.first.as<std::string>() == "version") {
                        /* Ignore here. */
                    } else if(node_it.first.as<std::string>() == "system") {
                        Log::dbg << "=== Loading system configuration...\n";
                        getConfigParam("n_agents", node_it.second, n_agents);
                        YAML::Node time_node = node_it.second["time"];
                        if(time_node.IsDefined()) {
                            getConfigParam("duration", time_node, duration);
                            if(time_node["type"].IsDefined()) {
                                if(time_node["type"].as<std::string>() == "julian_days") {
                                    float sec  = time_node["sec"].as<float>();
                                    float min  = time_node["min"].as<float>();
                                    float hour = time_node["hour"].as<float>();
                                    float days = time_node["days"].as<float>();
                                    time_step  = days + (hour / 24.f) + (min / 1440.f) + (sec / 86400.f);
                                    time_type = TimeValueType::JULIAN_DAYS;
                                    Log::dbg << "Config. parameter \'time_type\' is set to JULIAN_DAYS.\n";
                                    getConfigParam("start_epoch", time_node, start_epoch);
                                    Log::dbg << "Config. parameter \'time_step\' is set to " << time_step << " => "
                                        << days << " days, " << hour << " hours, " << min << " min, " << sec << " sec.\n";
                                } else if(time_node["type"].as<std::string>() == "arbitrary") {
                                    time_type = TimeValueType::ARBITRARY;
                                    Log::dbg << "Config. param \'time_type\' is set to ARBITRARY.\n";
                                    start_epoch = 0.0;
                                    getConfigParam("value", time_node, time_step);
                                } else if(time_node["type"].as<std::string>() == "seconds") {
                                    time_type = TimeValueType::SECONDS;
                                    Log::dbg << "Config. param \'time_type\' is set to SECONDS.\n";
                                    start_epoch = 0.0;
                                    getConfigParam("sec", time_node, time_step);
                                } else {
                                    throw std::runtime_error("Unrecognized time type in configuration file.");
                                }
                            }
                        }

                    } else if(node_it.first.as<std::string>() == "graphics") {
                        Log::dbg << "=== Loading graphics configuration...\n";
                        getConfigParam("win_width", node_it.second, win_width);
                        getConfigParam("win_height", node_it.second, win_height);
                        getConfigParam("agent_size", node_it.second, agent_size);
                        getConfigParam("font_size", node_it.second, fnt_size);

                    } else if(node_it.first.as<std::string>() == "agent") {
                        Log::dbg << "=== Loading agent configuration...\n";
                        getConfigParam("activity_size", node_it.second, activity_size);
                        getConfigParam("energy_generation", node_it.second, agent_energy_generation_rate);
                        getConfigParam("planning_window", node_it.second, agent_planning_window);
                        getConfigParam("max_tasks", node_it.second, max_tasks);
                        getConfigParam("max_task_duration", node_it.second, max_task_duration);

                        if(node_it.second["instrument"].IsDefined()) {
                            YAML::Node instrument_node = node_it.second["instrument"];
                            getConfigParam("aperture", instrument_node, agent_aperture_min, agent_aperture_max);
                            getConfigParam("energy", instrument_node, instrument_energy_min, instrument_energy_max);
                            getConfigParam("storage", instrument_node, instrument_storage_min, instrument_storage_max);
                        } else {
                            throw std::runtime_error("Agent instrument model parameters have not been provided.");
                        }

                        if(node_it.second["link"].IsDefined()) {
                            YAML::Node link_node = node_it.second["link"];
                            getConfigParam("range", link_node, agent_range_min, agent_range_max);
                            getConfigParam("datarate", link_node, agent_datarate_min, agent_datarate_max);
                            getConfigParam("energy_tx", link_node, link_tx_energy_rate);
                            getConfigParam("energy_rx", link_node, link_rx_energy_rate);
                            getConfigParam("allow_during_capture", link_node, link_allow_during_capture);
                        } else {
                            throw std::runtime_error("Agent link model parameters have not been provided.");
                        }

                        if(node_it.second["motion"].IsDefined()) {
                            YAML::Node motion_node = node_it.second["motion"];
                            if(motion_node["type"].IsDefined()) {
                                if(motion_node["type"].as<std::string>() == "orbital") {
                                    motion_model = AgentMotionType::ORBITAL;
                                    Log::dbg << "Motion \'type\' is set to: ORBITAL. Trajectories will be modelled in 3D.\n";
                                } else if(motion_node["type"].as<std::string>() == "linear_bounce") {
                                    motion_model = AgentMotionType::LINEAR_BOUNCE;
                                    Log::dbg << "Motion \'type\' is set to: LINEAR_BOUNCE. Trajectories will be modelled in 2D.\n";
                                } else if(motion_node["type"].as<std::string>() == "linear_infinite") {
                                    motion_model = AgentMotionType::LINEAR_INFINITE;
                                    Log::dbg << "Motion \'type\' is set to: LINEAR_INFINITE. Trajectories will be modelled in 2D.\n";
                                    Log::warn << "Motion model LINEAR_INFINITE is not fully implemented and tested.\n";
                                } else if(motion_node["type"].as<std::string>() == "sinusoidal") {
                                    motion_model = AgentMotionType::SINUSOIDAL;
                                    Log::dbg << "Motion \'type\' is set to: SINUSOIDAL. Trajectories will be modelled in 2D.\n";
                                    Log::warn << "Motion model SINUSOIDAL is not fully implemented and tested.\n";
                                }
                                if(motion_model == AgentMotionType::ORBITAL) {
                                    switch(time_type) {
                                        case TimeValueType::ARBITRARY:
                                            Log::err << "Time type is set to ARBITRARY but motion model is set to ORBITAL. ";
                                            Log::err << "This will result in undefined behavior.\n";
                                            break;
                                        case TimeValueType::SECONDS:
                                            Log::warn << "Time type is set to SECONDS and motion model is set to ORBITAL. ";
                                            Log::warn << "This mode has not been fully tested.\n";
                                            break;
                                        case TimeValueType::JULIAN_DAYS: /* This is OK. */
                                            break;
                                    }
                                } else if(motion_model != AgentMotionType::ORBITAL && time_type == TimeValueType::JULIAN_DAYS) {
                                    Log::dbg << "Time type is set to JULIAN_DAYS but motion model is not set to ORBITAL. This might be unexpected.\n";
                                }
                            } else {
                                throw std::runtime_error("Motion model type has not been provided.");
                            }
                            switch(motion_model) {
                                case AgentMotionType::ORBITAL:
                                    if(getConfigParam("altitude", motion_node, orbp_sma_min, orbp_sma_max)) {
                                        orbp_sma_max *= 1e3f;    /* Convert km to m. */
                                        orbp_sma_min *= 1e3f;    /* Convert km to m. */
                                        orbp_sma_max += earth_wgs84_a;
                                        orbp_sma_min += earth_wgs84_a;
                                    }
                                    getConfigParam("max_ecc", motion_node, orbp_ecc_max);
                                    getConfigParam("inc", motion_node, orbp_inc_min, orbp_inc_max);
                                    getConfigParam("argp", motion_node, orbp_argp_min, orbp_argp_max);
                                    getConfigParam("raan", motion_node, orbp_raan_min, orbp_raan_max);
                                    getConfigParam("init_ma", motion_node, orbp_init_ma_min, orbp_init_ma_max);
                                    break;
                                default:
                                    getConfigParam("speed", motion_node, agent_speed);
                                    break;
                            }
                        } else {
                            throw std::runtime_error("Agent motion model parameters have not been provided.");
                        }

                        if(node_it.second["ga_scheduler"].IsDefined()) {
                            YAML::Node gasch_node = node_it.second["ga_scheduler"];
                            getConfigParam("generations", gasch_node, ga_generations);
                            getConfigParam("timeout", gasch_node, ga_timeout);
                            getConfigParam("min_improvement_rate", gasch_node, ga_min_improvement_rate);
                            getConfigParam("population_size", gasch_node, ga_population_size);
                            getConfigParam("mutation_rate", gasch_node, ga_mutation_rate);
                            if(gasch_node["crossover"].IsDefined() && gasch_node["crossover"]["type"].IsDefined()) {
                                if(gasch_node["crossover"]["type"].as<std::string>() == "uniform") {
                                    ga_crossover_op = GASCrossoverOp::UNIFORM;
                                } else if(gasch_node["crossover"]["type"].as<std::string>() == "single") {
                                    ga_crossover_op = GASCrossoverOp::SINGLE_POINT;
                                } else if(gasch_node["crossover"]["type"].as<std::string>() == "multiple") {
                                    ga_crossover_op = GASCrossoverOp::MULIPLE_POINT;
                                    getConfigParam("n_points", gasch_node["crossover"], ga_crossover_points);
                                } else {
                                    Log::warn << "GA Scheduler configuration missing or wrong: crossover operator type, setting to SINGLE_POINT.\n";
                                    ga_crossover_op = GASCrossoverOp::SINGLE_POINT;
                                }
                            } else {
                                throw std::runtime_error("GA Scheduler crossover operator options have not been provided");
                            }
                            if(gasch_node["parent_sel"].IsDefined() && gasch_node["parent_sel"]["type"].IsDefined()) {
                                if(gasch_node["parent_sel"]["type"].as<std::string>() == "tournament") {
                                    ga_parentsel_op = GASSelectionOp::TOURNAMENT;
                                    getConfigParam("k", gasch_node["parent_sel"], ga_tournament_k);
                                } else if(gasch_node["parent_sel"]["type"].as<std::string>() == "fitness_proportionate") {
                                    ga_parentsel_op = GASSelectionOp::FITNESS_PROPORTIONATE_ROULETTE_WHEEL;
                                } else if(gasch_node["parent_sel"]["type"].as<std::string>() == "stochastic_universal") {
                                    ga_parentsel_op = GASSelectionOp::STOCHASTIC_UNIVERSAL;
                                    Log::err << "GA Scheduler parent selection operator STOCHASTIC_UNIVERSAL is not implemented.\n";
                                } else if(gasch_node["parent_sel"]["type"].as<std::string>() == "elitist") {
                                    ga_parentsel_op = GASSelectionOp::ELITIST;
                                    Log::err << "GA Scheduler parent selection operator ELITIST is not implemented.\n";
                                } else {
                                    ga_parentsel_op = GASSelectionOp::FITNESS_PROPORTIONATE_ROULETTE_WHEEL;
                                    Log::warn << "GA Scheduler configuration missing or wrong: parent selection operator, setting to FITNESS_PROPORTIONATE_ROULETTE_WHEEL.\n";
                                }
                            } else {
                                throw std::runtime_error("GA Scheduler parent selection operator options have not been provided.");
                            }
                            if(gasch_node["environ_sel"].IsDefined() && gasch_node["environ_sel"]["type"].IsDefined()) {
                                if(gasch_node["environ_sel"]["type"].as<std::string>() == "elitist") {
                                    ga_environsel_op = GASSelectionOp::ELITIST;
                                } else if(gasch_node["environ_sel"]["type"].as<std::string>() == "generational") {
                                    ga_environsel_op = GASSelectionOp::GENERATIONAL;
                                } else if(gasch_node["environ_sel"]["type"].as<std::string>() == "truncation") {
                                    ga_environsel_op = GASSelectionOp::TRUNCATION;
                                } else {
                                    ga_environsel_op = GASSelectionOp::ELITIST;
                                    Log::warn << "GA Scheduler configuration missing or wrong: environment combination operator, setting to ELITIST.\n";
                                }
                            } else {
                                throw std::runtime_error("GA Scheduler combination operator options have not been provided.");
                            }
                        }
                    } else if(node_it.first.as<std::string>() == "environment") {
                        Log::dbg << "=== Loading environment configuration...\n";
                        getConfigParam("model_unity_size", node_it.second, model_unity_size);
                        getConfigParam("world_width", node_it.second, world_width);
                        getConfigParam("world_height", node_it.second, world_height);
                        getConfigParam("max_revisit_time", node_it.second, max_revisit_time);
                        getConfigParam("target_revisit_time", node_it.second, target_revisit_time);
                        getConfigParam("min_payoff", node_it.second, min_payoff);
                        getConfigParam("max_payoff", node_it.second, max_payoff);
                    } else {
                        Log::warn << "=== Skipping unrecognized configuration category \'" << node_it.first.as<std::string>() << "\'.\n";
                    }
                }
            } catch(const std::exception& e) {
                Log::err << "Error loading configuration from \'PROJECT_ROOT/" << ("conf/" + opt_val) << "\'.\n";
                Log::err << e.what() << "\n";
            }
        }
    }

    VirtualTime::doInit(start_epoch);
    if(Config::motion_model == AgentMotionType::ORBITAL) {
        Config::time_type = TimeValueType::JULIAN_DAYS;
    } else {
        Config::time_type = TimeValueType::ARBITRARY;
    }
}

void Config::logProxyErr(std::string s)
{
    Log::err << s;
}

void Config::logProxyWarn(std::string s)
{
    Log::warn << s;
}

void Config::logProxyDbg(std::string s)
{
    Log::dbg << s;
}

int randomProxy(int a, int b)
{
    return Random::getUi(a, b);
}

float randomProxy(float a, float b)
{
    return Random::getUf(a, b);
}
