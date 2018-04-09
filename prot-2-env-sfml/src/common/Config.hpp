/***********************************************************************************************//**
 *  Program configuration values.
 *  @class      Config
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "prot.hpp"

class ColorGradient;

class Config
{
public:
    /* General constants: */
    static constexpr const double pi = std::acos(-1.0);     /**< The number pi.                 */

    /* Configuration constants: */
    static const unsigned int win_width  = 1200;            /**< Default window width.          */
    static const unsigned int win_height = 1000;            /**< Default window height.         */
    static const unsigned int world_width  = 1200;          /**< Default window width.          */
    static const unsigned int world_height = 1000;          /**< Default window height.         */
    static const unsigned int model_unity_size = 05;        /**< Size of the model unity.       */
    static const unsigned int agent_size = 14;              /**< Size of an agent view.         */
    static const unsigned int n_agents = 10;                /**< Total number of agents.        */
    static constexpr const float agent_swath_min = 40.f;    /**< Minimum swath for agents.      */
    static constexpr const float agent_swath_max = 80.f;    /**< Maximum swath for agents.      */
    static constexpr const float agent_range_min = 50.f;    /**< Minimum range for agents.      */
    static constexpr const float agent_range_max = 90.f;    /**< Maximum range for agents.      */
    static constexpr const float agent_speed = 2.f;         /**< Distance per time unit.        */
    static constexpr const float time_step = 2.f;           /**< Units of time per step.        */
    static const unsigned int agent_propagation_size = 3e4; /**< Max. # propagated states.      */
    static constexpr const float capacity_restore = 0.01f;  /**< Unit of capacity restoring.    */
    static constexpr const float capacity_consume = 0.02f;  /**< Unit of capacity consumption.  */
    static constexpr const float max_capacity = 10.f;       /**< Max. resource capacity.        */
    static constexpr const float max_revisit_time = 6e3;    /**< Units of time.                 */

    /* Format and colors: */
    static sf::Font fnt_monospace;
    static sf::Font fnt_normal;
    static sf::Font fnt_awesome;
    static sf::Color color_orange;
    static sf::Color color_dark_green;
    static ColorGradient color_gradient_rgb;

    static const unsigned int fnt_size = 24;
};


#endif /* CONFIG_HPP */
