/***********************************************************************************************//**
 *  Program configuration values.
 *  @class      Config
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-nov-16
 *  @version    0.2
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Config.hpp"
#include "ColorGradient.hpp"

/*  NOTE:
 *  Some of the following are variables might be ODR-used somewhere and, therefore, could need a
 *  definition in the source. To prevent unexpected behaviour in the compilation process, all have
 *  been explicitly defined below (even variables that could be initialized in headers).
 *  For more information, please refer to the pages below:
 *    - Static const integral variables:    https://stackoverflow.com/a/45719501/1876268
 *    - One-Definition-Rule definition:     https://en.cppreference.com/w/cpp/language/definition
 **/

/* General constants: */
const double  Config::pi = std::acos(-1.0);

/* Configuration constants: */
const unsigned int  Config::win_width  = 1800;
const unsigned int  Config::win_height =  900;
const unsigned int  Config::world_width  = 1800;
const unsigned int  Config::world_height =  900;
const unsigned int  Config::model_unity_size = 10;
const unsigned int  Config::agent_size = 14;
const unsigned int  Config::n_agents =  1;
const double        Config::start_epoch = 2451545.0;
const double        Config::time_step = 10.0 / 86400.0;
const float         Config::max_revisit_time = 0.7f;
const float         Config::target_revisit_time = 0.2f;
const float         Config::min_payoff = 1e-3f;
const float         Config::max_payoff = 1.f;

/* Earth WGS84 parameters: */
const double Config::earth_wgs84_a = 6378137.0;
const double Config::earth_wgs84_b = 6356752.314245;
const double Config::earth_wgs84_e = 0.08181919;
const double Config::earth_mu = 3.986004419e14;

/* Agent parametrization: */
const float             Config::agent_aperture_min = 60.f;
const float             Config::agent_aperture_max = 120.f;
const float             Config::agent_range_min = 50.f;
const float             Config::agent_range_max = 90.f;
const float             Config::agent_datarate_min = 0.1f;
const float             Config::agent_datarate_max = 0.2f;
const float             Config::agent_speed =  4.f;
const unsigned int      Config::agent_planning_window = 1080; /* Steps (540 ~= 1 orbit). */
const float             Config::activity_size = 0.01f;
const AgentMotionType   Config::motion_model = AgentMotionType::ORBITAL;
TimeValueType           Config::time_type;

/* Resource consumptions and capacities: */
/* -- Energy: */
const float     Config::agent_energy_generation_rate = -764.f;
const float     Config::instrument_energy_min = 900.f;
const float     Config::instrument_energy_max = 1200.f;
const float     Config::link_tx_energy_rate = 0.5f;
const float     Config::link_rx_energy_rate = 0.05f;
/* -- Storage: */
const float     Config::instrument_storage_min = 0.01f;
const float     Config::instrument_storage_max = 0.01f;

/* Format and colors: */
const unsigned int Config::fnt_size = 24;
sf::Font        Config::fnt_monospace;
sf::Font        Config::fnt_normal;
sf::Font        Config::fnt_awesome;
sf::Color       Config::color_orange;
sf::Color       Config::color_dark_green;
ColorGradient   Config::color_gradient_rgb;
ColorGradient   Config::color_gradient_rbg;
ColorGradient   Config::color_gradient_krbg;
ColorGradient   Config::color_gradient_rainbow;
ColorGradient   Config::color_gradient_1;
ColorGradient   Config::color_gradient_blue;

/* Scheduling hard constraints: */
const unsigned int  Config::max_tasks = 25;
const double        Config::max_task_duration = 7.0 * 60.0 / 86400.0;    /* 7 min. in JD */

/* Genetic Algorithm configuration: */
const unsigned int      Config::ga_generations = 10000;
const unsigned int      Config::ga_timeout = 1000;
const float             Config::ga_min_improvement_rate = 0.01f;
const unsigned int      Config::ga_population_size = 500;
unsigned int            Config::ga_crossover_points = 5;
const unsigned int      Config::ga_tournament_k = 2;
const float             Config::ga_mutation_rate = 0.2f;
const GASCrossoverOp    Config::ga_crossover_op = GASCrossoverOp::MULIPLE_POINT;
const GASSelectionOp    Config::ga_parentsel_op = GASSelectionOp::TOURNAMENT;
const GASSelectionOp    Config::ga_environsel_op = GASSelectionOp::ELITIST;

/* Global values: */
std::string Config::root_path;
