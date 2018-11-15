/***********************************************************************************************//**
 *  Program configuration values.
 *  @class      Config
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *              Marc Closa (MC), marc.closa.tarres@alu-etsetb.upc.edu
 *  @date       2018-nov-15
 *  @version    0.2
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "prot.hpp"
#include "common_enum_types.hpp"
#include "GASOperators.hpp"

class ColorGradient;

class Config
{
public:
    /* General constants: */
    static const double pi;               /**< The number pi.                 */

    /* Configuration constants: */
    static const unsigned int win_width;            /**< Default window width. */
    static const unsigned int win_height;           /**< Default window height. */
    static const unsigned int world_width;          /**< Default window width. */
    static const unsigned int world_height;         /**< Default window height. */
    static const unsigned int model_unity_size;     /**< Size of the model unity. */
    static const unsigned int agent_size;           /**< Size of an agent view. */
    static const unsigned int n_agents;             /**< Total number of agents. */
    static const double start_epoch;                /**< Start epoch (in J2000) */
    static const double time_step;                  /**< Units of time per step. */
    static const float max_revisit_time;            /**< Units of time. */
    static const float target_revisit_time;         /**< Units of time. */
    static const float min_payoff;                  /**< Unit-less (payoff RT < target) */
    static const float max_payoff;                  /**< Unit-less (payoff RT > max.) */

    /* Earth WGS84 parameters: */
    static const double earth_wgs84_a;        /**< Semi-major axis of WGS84 ellipsoid (in meters). */
    static const double earth_wgs84_b;        /**< Semi-minor axis of WGS84 ellipsoid (in meters). */
    static const double earth_wgs84_e;        /**< Eccentricity of WGS84 ellipsoid. */
    static const double earth_mu;             /**< Earth's gravitational constant. */

    /* Agent parametrization: */
    static const float agent_aperture_min;              /**< Min. aperture for instruments. */
    static const float agent_aperture_max;              /**< Max. aperture for instruments. */
    static const float agent_range_min;                 /**< Minimum range for links. */
    static const float agent_range_max;                 /**< Maximum range for links. */
    static const float agent_datarate_min;              /**< Minimum range for links. */
    static const float agent_datarate_max;              /**< Maximum range for links. */
    static const float agent_speed;                     /**< Distance per time unit. */
    static const unsigned int agent_planning_window;    /**< Steps. 540 ~= 1 orbit. */
    static const float activity_size;                   /**< Size of a single agent msg. */
    static const AgentMotionType motion_model;          /**< Type of trajectory and motion model. */
    static TimeValueType time_type;                     /**< Type of units in time magnitudes. */

    /* Resource consumptions and capacities: */
    /* -- Energy: */
    static const float agent_energy_generation_rate;
    static const float instrument_energy_min;
    static const float instrument_energy_max;
    static const float link_tx_energy_rate;
    static const float link_rx_energy_rate;
    /* -- Storage: */
    static const float instrument_storage_min;
    static const float instrument_storage_max;

    /* Format and colors: */
    static sf::Font fnt_monospace;
    static sf::Font fnt_normal;
    static sf::Font fnt_awesome;
    static sf::Color color_orange;
    static sf::Color color_dark_green;
    static ColorGradient color_gradient_rgb;
    static ColorGradient color_gradient_rbg;
    static ColorGradient color_gradient_krbg;
    static ColorGradient color_gradient_rainbow;
    static ColorGradient color_gradient_1;
    static ColorGradient color_gradient_blue;
    static const unsigned int fnt_size;

    /* Scheduling hard constraints: */
    static const unsigned int max_tasks;
    static const double max_task_duration;

    /* Genetic Algorithm configuration: */
    static const unsigned int ga_generations;       /**< Max. absolute number of iterations. */
    static const unsigned int ga_timeout;           /**< Max. number of iterations without improvement. */
    static const float ga_min_improvement_rate;     /**< Min. fitness/iteration rate to complete. */
    static const unsigned int ga_population_size;   /**< Number of individuals in the population. */
    static unsigned int ga_crossover_points;        /**< Points of chromosome crossover. */
    static const unsigned int ga_tournament_k;      /**< Parameter K in tournament selection operator. */
    static const float ga_mutation_rate;            /**< Mutation probability. */
    static const GASCrossoverOp ga_crossover_op;    /**< Crossover operator. */
    static const GASSelectionOp ga_parentsel_op;    /**< Parent selection operator. */
    static const GASSelectionOp ga_environsel_op;   /**< Environment/combination operator. */

    /* Global values: */
    static std::string root_path;   /**< Root path of the project. */
};


#endif /* CONFIG_HPP */
