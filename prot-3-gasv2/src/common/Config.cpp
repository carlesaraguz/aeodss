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
#include "PayoffFunctions.hpp"

CREATE_LOGGER(Config)

#define CONF_VERSION  4

/*  NOTE:
 *  Some of the following are variables might be ODR-used somewhere and, therefore, could need a
 *  definition in the source. To prevent unexpected behaviour in the compilation process, all have
 *  been explicitly defined below (even variables that could be initialized in headers).
 *  For more information, please refer to the pages below:
 *    - Static const integral variables:    https://stackoverflow.com/a/45719501/1876268
 *    - One-Definition-Rule definition:     https://en.cppreference.com/w/cpp/language/definition
 **/

/* General constants: */
const long double Config::pi = std::acos(-1.0);

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
bool            Config::enable_graphics = true;

/* Concurrency settings: */
bool            Config::parallel_nested = true;
unsigned int    Config::parallel_planners = 1;

/* System goals and payoff model: */
double          Config::goal_target = 0.5;      /* 12 hours.   */
double          Config::goal_min = 0.2;         /* 4.8 hours.  */
double          Config::goal_max = 0.7;         /* 16.8 hours. */
PayoffModel     Config::payoff_model = PayoffModel::SIGMOID;
float           Config::payoff_mid = 0.5f;
float           Config::payoff_steepness = 20.f;
float           Config::payoff_slope = 1.f;

/* Earth WGS84 parameters: */
const long double    Config::earth_radius  = 6371000.0L;
const long double    Config::earth_wgs84_a = 6378137.0L;
const long double    Config::earth_wgs84_b = 6356752.314245L;
const long double    Config::earth_wgs84_e = 0.08181919L;
const long double    Config::earth_mu = 3.986004419e14L;

/* Agent parametrization: */
float           Config::agent_aperture_min = 60.f;
float           Config::agent_aperture_max = 120.f;
float           Config::agent_range_min = 50.f;
float           Config::agent_range_max = 90.f;
float           Config::agent_datarate_min = 100.f;
float           Config::agent_datarate_max = 200.f;
bool            Config::link_allow_during_capture = false;
float           Config::agent_speed =  4.f;
unsigned int    Config::agent_planning_window = 1080;   /* Steps (540 ~= 1 orbit). */
unsigned int    Config::agent_replanning_window = 100;  /* Steps (540 ~= 1 orbit). */
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
float Config::link_tx_energy_rate = 0.005f;
float Config::link_rx_energy_rate = 0.0005f;
float Config::link_reserved_capacity = 0.1f;

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
unsigned int    Config::knowledge_base_size = 10000; /* Virtually unlimited. */
unsigned int    Config::max_tasks = 25;
unsigned int    Config::max_task_duration = 10;
float           Config::min_payoff = 0.f;
float           Config::max_payoff = 1.f;
unsigned int    Config::activity_confirm_window = 10; /* 1.2h in JD. */
float           Config::confidence_mod_exp = 2.f;
float           Config::utility_floor = 0.f;
float           Config::utility_k = 10.f;   /* Should be higher than 10. */
float           Config::utility_unknown = 0.75f;
float           Config::utility_weight = 0.5f;
float           Config::decay_weight = 0.5f;

/* Genetic Algorithm configuration: */
Aggregate       Config::ga_payoff_aggregate = Aggregate::SUM_VALUE;
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
float           Config::ga_payoff_k = 5.f;
float           Config::ga_confidence_th = 0.5f;

/* Global values: */
bool        Config::create_data_dirname = true;
bool        Config::load_agents_from_yaml = false;
std::string Config::system_yml;
std::string Config::simulation_name;
std::string Config::root_path;
std::string Config::data_path;
std::string Config::conf_file;
std::string Config::tle_file;
SandboxMode Config::mode = SandboxMode::SIMULATE;
bool        Config::shared_memory = true;
bool        Config::simple_log = false;
bool        Config::verbosity = true;
int         Config::interpos = 2;


void Config::loadCmdArgs(int argc, char** argv)
{
    data_path = "";
    std::string opt, opt_val;
    bool force_graphics = false;
    bool override_graphics_value = false;
    for(int cmd_idx = 1; cmd_idx < argc; cmd_idx++) {
        opt = argv[cmd_idx];
        if(opt == "-h" || opt == "--help" || opt == "-help") {
            Log::dbg << "Printing help. Argument options:\n";
            Log::dbg << "            -h --help  Shows this help.\n";
            Log::dbg << "                  -tp  Enters TEST_PAYOFF mode. Generates revisit time values and prints the resulting payoffs.\n";
            Log::dbg << "             --random  Enters RANDOM mode. Simulates with random agent behaviour.\n";
            Log::dbg << "   --parse-tle <file>  Enters PARSE_TLE_FILE mode. Generates a system.yml from a TLE collection file. Does not simulate.\n";
            Log::dbg << "            -f <file>  Reads configuration file. Output is stored in an auto-generated folder inside \'data\'.\n";
            Log::dbg << "                       The autogenerated results folder is postfixed to the simulation \'name\' (if given).\n";
            Log::dbg << "             -d <dir>  Defines an output directory (relative to bash execution, not binary). \n";
            Log::dbg << "                       This overrides the option \'name\' in the config file.\n";
            Log::dbg << "            -l <path>  Loads \'n_agents\' from the file indicated in <path>. \n";
            Log::dbg << "                       The path must be the location of a YAML file that has the structure of \'system.yml\'. \n";
            Log::dbg << "                       If the `-l` option is not set, the program generates one \'system.yml\' in the results folder.\n";
            Log::dbg << "              -g[0|1]  Overrides `graphics.enable` value: -g0 = graphics disabled.\n";
            Log::dbg << "  --dbg-rootdir <dir>  Overrides the root path with the given one (for debug purposes only).\n";
            Log::dbg << "         --simple-log  Does not print logs with colors.\n";

        } else if(opt == "-tp") {
            /* Will enter in 'test payoff' mode. */
            Log::dbg << "TEST_PAYOFF mode selected.\n";
            mode = SandboxMode::TEST_PAYOFF;

        } else if(opt == "--parse-tle" && (cmd_idx + 1) < argc) {
            tle_file = argv[cmd_idx + 1];
            mode = SandboxMode::PARSE_TLE_FILE;
            Log::dbg << "PARSE_TLE_FILE mode selected (\'" << tle_file << "\').\n";

        } else if(opt == "--random") {
            /* Will enter in 'test payoff' mode. */
            Log::dbg << "RANDOM mode selected.\n";
            Log::dbg << "=== Overwritting configuration:\n";
            mode = SandboxMode::RANDOM;
            simulation_name += "_rand";
            agent_range_min = 1.f;
            Log::dbg << " -- Config. parameter \'agent_range_min\' is set to: " << agent_range_min << "\n";
            agent_range_max = 1.f;
            Log::dbg << " -- Config. parameter \'agent_range_max\' is set to: " << agent_range_max << "\n";
            agent_replanning_window = agent_planning_window + 1;
            Log::dbg << " -- Config. parameter \'replanning_window\' is set to: " << agent_replanning_window << "\n";
            knowledge_base_size = 0;
            Log::dbg << " -- Config. parameter \'knowledge_base_size\' is set to: " << knowledge_base_size << "\n";
            ga_generations = 0;
            Log::dbg << " -- Config. parameter \'ga_generations\' is set to: " << ga_generations << "\n";
            ga_timeout = 0;
            Log::dbg << " -- Config. parameter \'ga_timeout\' is set to: " << ga_timeout << "\n";
            ga_population_size = 500;
            Log::dbg << " -- Config. parameter \'ga_population_size\' is set to: " << ga_population_size << "\n";

        } else if(opt == "--simple-log") {
            /* Will enter in 'test payoff' mode. */
            simple_log = true;
            Log::dbg << "Log will not print colors\n";

        } else if(opt == "-g0") {
            /* Will enter in 'test payoff' mode. */
            force_graphics = true;
            override_graphics_value = false;

        } else if(opt == "-shm1") {
            /* Will enter in 'test payoff' mode. */
            shared_memory = true;

        } else if(opt == "-shm0") {
            /* Will enter in 'test payoff' mode. */
            shared_memory = false;
            Log::warn << "Shared memory regions have been disabled. Information will be replicated.\n";
            Log::warn << "NOTE: This is a more representative case, but does not provide different "
                "results (at behavioural and functional levels).\n";

        } else if(opt == "-g1") {
            /* Will enter in 'test payoff' mode. */
            force_graphics = true;
            override_graphics_value = true;

        } else if(opt == "--dbg-rootdir" && (cmd_idx + 1) < argc) {
            root_path = argv[cmd_idx + 1];
            Log::warn << "(DEBUG) Root path set to: " << root_path << "\n";

        } else if(opt == "-d" && (cmd_idx + 1) < argc) {
            simulation_name = argv[cmd_idx + 1];
            if(simulation_name[simulation_name.length() - 1] != '/') {
                simulation_name += "/";
            }
            Log::dbg << "Simulation output path has been set to: " << root_path << simulation_name << "\n";
            create_data_dirname = false;

        } else if(opt == "-l" && (cmd_idx + 1) < argc) {
            system_yml = argv[cmd_idx + 1];
            load_agents_from_yaml = true;
            Log::dbg << "Agent configuration will be loaded from: " << system_yml << "\n";

        } else if(opt == "-f" && (cmd_idx + 1) < argc) {
            opt_val = argv[cmd_idx + 1];
            try {
                YAML::Node conf = YAML::LoadFile(root_path + "conf/" + opt_val);
                conf_file = root_path + "conf/" + opt_val;
                Log::dbg << "Loading configuration from \'" << root_path << ("conf/" + opt_val) << "\'.\n";
                if(conf["version"].IsDefined()) {
                    unsigned int conf_ver = conf["version"].as<unsigned int>();
                    if(conf_ver < CONF_VERSION) {
                        Log::err << "Configuration file version " << conf_ver << " is older than the implementation.\n";
                    } else if(conf_ver > CONF_VERSION) {
                        Log::err << "Unexpected config. file version " << conf_ver
                            << ". Implementation only supports versions <= " << CONF_VERSION << ".\n";
                        throw std::runtime_error("Unsupported configuration file. Wrong version specs.");
                    }
                } else {
                    Log::err << "Config. file version has not been defined. Aborting.\n";
                    throw std::runtime_error("Unknown configuration file version.");
                }
                for(auto node_it : conf) {
                    if(node_it.first.as<std::string>() == "version") {
                        /* Ignore here. */
                    } else if(node_it.first.as<std::string>() == "system") {
                        Log::dbg << "=== Loading system configuration...\n";
                        if(create_data_dirname) {
                            getConfigParam("name", node_it.second, simulation_name);
                        }
                        getConfigParam("n_agents", node_it.second, n_agents);
                        getConfigParam("verbosity", node_it.second, verbosity);
                        getConfigParam("interpos", node_it.second, interpos);
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
                                    Log::dbg << " -- Config. parameter \'time_type\' is set to JULIAN_DAYS.\n";
                                    getConfigParam("start_epoch", time_node, start_epoch);
                                    Log::dbg << " -- Config. parameter \'time_step\' is set to " << time_step << " => "
                                        << days << " days, " << hour << " hours, " << min << " min, " << sec << " sec.\n";
                                } else if(time_node["type"].as<std::string>() == "arbitrary") {
                                    time_type = TimeValueType::ARBITRARY;
                                    Log::dbg << " -- Config. param \'time_type\' is set to ARBITRARY.\n";
                                    start_epoch = 0.0;
                                    getConfigParam("value", time_node, time_step);
                                } else if(time_node["type"].as<std::string>() == "seconds") {
                                    time_type = TimeValueType::SECONDS;
                                    Log::dbg << " -- Config. param \'time_type\' is set to SECONDS.\n";
                                    start_epoch = 0.0;
                                    getConfigParam("sec", time_node, time_step);
                                } else {
                                    throw std::runtime_error("Unrecognized time type in configuration file.");
                                }
                            }
                        }
                        YAML::Node parallel_node = node_it.second["parallel"];
                        if(parallel_node.IsDefined()) {
                            getConfigParam("nested", parallel_node, parallel_nested);
                            getConfigParam("planners", parallel_node, parallel_planners);
                            if(parallel_planners == 0) {
                                parallel_planners = 1;
                            }
                        }

                    } else if(node_it.first.as<std::string>() == "graphics") {
                        Log::dbg << "=== Loading graphics configuration...\n";
                        getConfigParam("enable", node_it.second, enable_graphics);
                        if(force_graphics) {
                            enable_graphics = override_graphics_value;
                        }
                        if(enable_graphics) {
                            getConfigParam("win_width", node_it.second, win_width);
                            getConfigParam("win_height", node_it.second, win_height);
                            getConfigParam("agent_size", node_it.second, agent_size);
                            getConfigParam("font_size", node_it.second, fnt_size);
                        } else {
                            Log::dbg << "Graphics are disabled\n";
                        }

                    } else if(node_it.first.as<std::string>() == "agent") {
                        Log::dbg << "=== Loading agent configuration...\n";
                        getConfigParam("activity_size", node_it.second, activity_size);
                        getConfigParam("energy_generation", node_it.second, agent_energy_generation_rate);
                        getConfigParam("planning_window", node_it.second, agent_planning_window);
                        getConfigParam("replanning_window", node_it.second, agent_replanning_window);
                        getConfigParam("confirm_window", node_it.second, activity_confirm_window);
                        getConfigParam("max_task_duration", node_it.second, max_task_duration);
                        Log::dbg << "Planning window is set to: " << VirtualTime::toString(agent_planning_window * time_step, false);
                        Log::dbg << ", " << agent_planning_window << " steps.\n";
                        Log::dbg << "Re-scheduling window is set to: " << VirtualTime::toString(agent_replanning_window * time_step, false);
                        Log::dbg << ", " << agent_replanning_window << " steps.\n";
                        Log::dbg << "Activity confirmation window is set to: " << VirtualTime::toString(activity_confirm_window * time_step, false);
                        Log::dbg << ", " << activity_confirm_window << " steps.\n";
                        if(max_task_duration <= 3) {
                            Log::warn << "Activity maximum duration is set to: " << VirtualTime::toString(max_task_duration * time_step, false);
                            Log::warn << ", " << max_task_duration << " steps.\n";
                        } else {
                            Log::dbg << "Activity maximum duration is set to: " << VirtualTime::toString(max_task_duration * time_step, false);
                            Log::dbg << ", " << max_task_duration << " steps.\n";
                        }
                        getConfigParam("max_tasks", node_it.second, max_tasks);
                        getConfigParam("knowledge_base_size", node_it.second, knowledge_base_size);
                        getConfigParam("min_payoff", node_it.second, min_payoff);

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
                            getConfigParam("reserved_capacity", link_node, link_reserved_capacity);
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
                        if(node_it.second["confidence"].IsDefined()) {
                            YAML::Node confidence_node = node_it.second["confidence"];
                            getConfigParam("exp", confidence_node, confidence_mod_exp);
                        } else {
                            throw std::runtime_error("Agent confidence update model parameters have not been provided.");
                        }
                        if(node_it.second["utility"].IsDefined()) {
                            YAML::Node utility_node = node_it.second["utility"];
                            getConfigParam("steepness", utility_node, utility_k);
                            getConfigParam("unknown", utility_node, utility_unknown);
                        } else {
                            throw std::runtime_error("Agent utility model parameters have not been provided.");
                        }
                        if(node_it.second["priority"].IsDefined()) {
                            YAML::Node priority_node = node_it.second["priority"];
                            getConfigParam("utility_floor", priority_node, utility_floor);
                            getConfigParam("utility_weight", priority_node, utility_weight);
                            getConfigParam("decay_weight", priority_node, decay_weight);
                            float wsum = decay_weight + utility_weight;
                            utility_weight /= wsum;
                            decay_weight /= wsum;
                            Log::dbg << "Decay weight after normalisation is: " << decay_weight << ".\n";
                            Log::dbg << "Utility weight after normalisation is: " << utility_weight << ".\n";
                        } else {
                            throw std::runtime_error("Activity priority model parameters have not been provided.");
                        }
                        if(node_it.second["ga_scheduler"].IsDefined()) {
                            YAML::Node gasch_node = node_it.second["ga_scheduler"];
                            getConfigParam("generations", gasch_node, ga_generations);
                            getConfigParam("timeout", gasch_node, ga_timeout);
                            getConfigParam("min_improvement_rate", gasch_node, ga_min_improvement_rate);
                            getConfigParam("population_size", gasch_node, ga_population_size);
                            getConfigParam("mutation_rate", gasch_node, ga_mutation_rate);
                            getConfigParam("payoff_k", gasch_node, ga_payoff_k);
                            getConfigParam("confidence_th", gasch_node, ga_confidence_th);
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
                            if(gasch_node["payoff_aggregation"].IsDefined()) {
                                if(gasch_node["payoff_aggregation"].as<std::string>() == "min") {
                                    ga_payoff_aggregate = Aggregate::MIN_VALUE;
                                    Log::dbg << " -- Config. parameter \'payoff_aggregate\' is set to: MIN.\n";
                                } else if(gasch_node["payoff_aggregation"].as<std::string>() == "max") {
                                    ga_payoff_aggregate = Aggregate::MAX_VALUE;
                                    Log::dbg << " -- Config. parameter \'payoff_aggregate\' is set to: MAX.\n";
                                } else if(gasch_node["payoff_aggregation"].as<std::string>() == "avg") {
                                    ga_payoff_aggregate = Aggregate::MEAN_VALUE;
                                    Log::dbg << " -- Config. parameter \'payoff_aggregate\' is set to: MEAN.\n";
                                } else if(gasch_node["payoff_aggregation"].as<std::string>() == "sum") {
                                    ga_payoff_aggregate = Aggregate::SUM_VALUE;
                                    Log::dbg << " -- Config. parameter \'payoff_aggregate\' is set to: SUM.\n";
                                } else {
                                    ga_payoff_aggregate = Aggregate::SUM_VALUE;
                                    Log::warn << "GA Scheduler configuration is wrong: payoff aggregation, setting to SUM.\n";
                                }
                            } else {
                                ga_payoff_aggregate = Aggregate::SUM_VALUE;
                                Log::warn << " -- Config. parameter \'payoff_aggregate\' is not defined. Default value: SUM.\n";
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
                        if(node_it.second["payoff"].IsDefined()) {
                            YAML::Node payoff_node = node_it.second["payoff"];
                            getConfigParam("goal_target", payoff_node, goal_target);
                            Log::dbg << "The system target revisit time is: " << VirtualTime::toString(goal_target, false, true) << ".\n";
                            if(payoff_node["type"].IsDefined()) {
                                if(payoff_node["type"].as<std::string>() == "sigmoid") {
                                    payoff_model = PayoffModel::SIGMOID;
                                    Log::dbg << " -- Config. parameter \'payoff.type\' is set to: SIGMOID.\n";
                                } else if(payoff_node["type"].as<std::string>() == "linear") {
                                    payoff_model = PayoffModel::LINEAR;
                                    Log::dbg << " -- Config. parameter \'payoff.type\' is set to: LINEAR.\n";
                                } else if(payoff_node["type"].as<std::string>() == "constant_slope") {
                                    payoff_model = PayoffModel::CONSTANT_SLOPE;
                                    Log::dbg << " -- Config. parameter \'payoff.type\' is set to: CONSTANT_SLOPE.\n";
                                } else if(payoff_node["type"].as<std::string>() == "quadratic") {
                                    payoff_model = PayoffModel::QUADRATIC;
                                    Log::dbg << " -- Config. parameter \'payoff.type\' is set to: QUADRATIC.\n";
                                } else {
                                    payoff_model = PayoffModel::SIGMOID;
                                    Log::warn << " -- Config. parameter \'payoff.type\' is not defined. Default value: SIGMOID.\n";
                                }
                            }
                            switch(payoff_model) {
                                case PayoffModel::SIGMOID:
                                    getConfigParam("steepness", payoff_node, payoff_steepness);
                                    break;
                                case PayoffModel::LINEAR:
                                    getConfigParam("payoff_mid", payoff_node, payoff_mid);
                                    getConfigParam("goal_min", payoff_node, goal_min);
                                    getConfigParam("goal_max", payoff_node, goal_max);
                                    Log::dbg << "The system min. revisit time is: " << VirtualTime::toString(goal_min, false, true) << ".\n";
                                    Log::dbg << "The system max. revisit time is: " << VirtualTime::toString(goal_max, false, true) << ".\n";
                                    break;
                                case PayoffModel::CONSTANT_SLOPE:
                                    getConfigParam("goal_min", payoff_node, goal_min);
                                    getConfigParam("slope", payoff_node, payoff_slope);
                                    Log::dbg << "The system min. revisit time is: " << VirtualTime::toString(goal_min, false, true) << ".\n";
                                    break;
                                case PayoffModel::QUADRATIC:
                                    getConfigParam("goal_min", payoff_node, goal_min);
                                    Log::dbg << "The system min. revisit time is: " << VirtualTime::toString(goal_min, false, true) << ".\n";
                                    break;
                            }
                        } else {
                            Log::err << "System goals and payoff model have not been defined.\n";
                            std::exit(-1);
                        }
                    } else {
                        Log::warn << "=== Skipping unrecognized configuration category \'" << node_it.first.as<std::string>() << "\'.\n";
                    }
                }
            } catch(const std::exception& e) {
                Log::err << "Error loading configuration from \'" << conf_file << "\'.\n";
                Log::err << e.what() << "\n";
                conf_file = "";
                std::exit(-1);
            }
            max_payoff = PayoffFunctions::payoff(Config::duration);
            Log::dbg << "The maximum payoff value for this configuration is: " << max_payoff << "\n";
        }
    }
    if(force_graphics) {
        enable_graphics = override_graphics_value;
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
