/***********************************************************************************************//**
 *  Program configuration values.
 *  @class      Config
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-20
 *  @version    0.1
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
    static constexpr const double pi = std::acos(-1.0);     /**< The number pi.                 */

    /* Configuration constants: */
    static const unsigned int win_width  = 1800;            /**< Default window width.          */
    static const unsigned int win_height =  900;            /**< Default window height.         */
    static const unsigned int world_width  = 1800;          /**< Default window width.          */
    static const unsigned int world_height =  900;          /**< Default window height.         */
    static const unsigned int model_unity_size = 10;        /**< Size of the model unity.       */
    static const unsigned int agent_size = 14;              /**< Size of an agent view.         */
    static const unsigned int n_agents =  5;                /**< Total number of agents.        */
    static constexpr const float time_step = 1.f;           /**< Units of time per step.        */
    static constexpr const float max_revisit_time = 2e3;    /**< Units of time.                 */
    static constexpr const float target_revisit_time = 1e3; /**< Units of time.                 */
    static constexpr const float min_payoff = 1e-3f;        /**< Unit-less (payoff RT < target) */
    static constexpr const float max_payoff = 1.f;          /**< Unit-less (payoff RT > max.)   */

    /* Agent parametrization: */
    static constexpr const float agent_swath_min = 100.f; //  70.f;   /**< Minimum swath for agents.      */
    static constexpr const float agent_swath_max = 200.f; // 132.f;   /**< Maximum swath for agents.      */
    static constexpr const float agent_range_min = 50.f;    /**< Minimum range for agents.      */
    static constexpr const float agent_range_max = 90.f;    /**< Maximum range for agents.      */
    static constexpr const float agent_datarate_min = 0.1f; /**< Minimum range for agents.      */
    static constexpr const float agent_datarate_max = 0.2f; /**< Maximum range for agents.      */
    static constexpr const float agent_speed = 10.f;        /**< Distance per time unit.        */
    static const unsigned int agent_planning_window = 3.5e2;  /**< Steps.                         */
    static constexpr const float activity_size = 0.01f;     /**< Size of a single agent msg.    */
    static const AgentMotionType motion_model = AgentMotionType::LINEAR_BOUNCE;
    /* Resource consumptions and capacities: */
    /* -- Energy: */
    static constexpr const float agent_energy_generation_rate = -0.01f;
    static constexpr const float instrument_energy_min = 0.01f;
    static constexpr const float instrument_energy_max = 0.02f;
    static constexpr const float link_tx_energy_rate = 0.5f;
    static constexpr const float link_rx_energy_rate = 0.05f;
    /* -- Storage: */
    static constexpr const float instrument_storage_min = 0.01f;
    static constexpr const float instrument_storage_max = 0.01f;

    /* Format and colors: */
    static sf::Font fnt_monospace;
    static sf::Font fnt_normal;
    static sf::Font fnt_awesome;
    static sf::Color color_orange;
    static sf::Color color_dark_green;
    static ColorGradient color_gradient_rgb;
    static ColorGradient color_gradient_rbg;
    static ColorGradient color_gradient_krbg;
    static ColorGradient color_gradient_1;
    static ColorGradient color_gradient_blue;
    static const unsigned int fnt_size = 24;

    /* Scheduling hard constraints: */
    static const unsigned int max_tasks = 20;
    static constexpr const float max_task_duration = 20.f;
    static constexpr const float task_startup_cost = 1.f;

    /* Genetic Algorithm configuration: */
    static const unsigned int ga_max_activities = 30;
    static const unsigned int ga_generations = 10000;
    static const unsigned int ga_timeout = 1000;
    static constexpr const float ga_min_improvement_rate = 0.01f;
    static const unsigned int ga_population_size = 500;
    static unsigned int ga_crossover_points;
    static const unsigned int ga_tournament_k = 2;
    static constexpr const float ga_mutation_rate = 0.2f;
    static const GASCrossoverOp ga_crossover_op = GASCrossoverOp::MULIPLE_POINT;
    static const GASSelectionOp ga_parentsel_op = GASSelectionOp::TOURNAMENT;
    static const GASSelectionOp ga_environsel_op = GASSelectionOp::ELITIST;
    static const unsigned int ga_thread_pool_size = 8;

    /* Global values: */
    static std::string root_path;
};


#endif /* CONFIG_HPP */
