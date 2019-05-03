/***********************************************************************************************//**
 *  Program configuration values.
 *  @class      Config
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-nov-23
 *  @version    0.3
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "prot.hpp"
#include <yaml-cpp/yaml.h>

#include "common_enum_types.hpp"
#include "GASOperators.hpp"

class ColorGradient;

class Config
{
public:
    /* General constants: */
    static const double pi;                     /**< The number pi. */

    /* Configuration constants: */
    static unsigned int win_width;              /**< Default window width. */
    static unsigned int win_height;             /**< Default window height. */
    static unsigned int world_width;            /**< Default window width. */
    static unsigned int world_height;           /**< Default window height. */
    static unsigned int model_unity_size;       /**< Size of the model unity. */
    static unsigned int agent_size;             /**< Size of an agent view. */
    static unsigned int n_agents;               /**< Total number of agents. */
    static double start_epoch;                  /**< Start epoch (in J2000) */
    static double duration;                     /**< Units of time. */
    static double time_step;                    /**< Units of time per step. */
    static bool enable_graphics;                /**< Whether to launch the graphical views. */

    /* Concurrency settings: */
    static bool parallel_nested;                /**< Wether to use OMP nested loops or not. */
    static unsigned int parallel_planners;      /**< Max number of parallel GAs planners. */

    /* System goals and payoff model: */
    static double goal_target;                  /**< Units of time. */
    static double goal_min;                     /**< Units of time. */
    static double goal_max;                     /**< Units of time. */
    static PayoffModel payoff_model;            /**< Model used to compute payoff. */
    static float payoff_mid;                    /**< Unit-less (payoff RT = target) */
    static float payoff_steepness;              /**< Steepness of logistic curve. */

    /* Earth WGS84 parameters: */
    static const double earth_radius;           /**< Standard radius of the Earth (in meters). */
    static const double earth_wgs84_a;          /**< Semi-major axis of WGS84 ellipsoid (in meters). */
    static const double earth_wgs84_b;          /**< Semi-minor axis of WGS84 ellipsoid (in meters). */
    static const double earth_wgs84_e;          /**< Eccentricity of WGS84 ellipsoid. */
    static const double earth_mu;               /**< Earth's gravitational constant. */

    /* Agent parametrization: */
    static float agent_aperture_min;            /**< Min. aperture for instruments. */
    static float agent_aperture_max;            /**< Max. aperture for instruments. */
    static float agent_range_min;               /**< Minimum range for links. */
    static float agent_range_max;               /**< Maximum range for links. */
    static float agent_datarate_min;            /**< Minimum range for links. */
    static float agent_datarate_max;            /**< Maximum range for links. */
    static bool link_allow_during_capture;      /**< Whether links and payloads can be enabled simultaneously. */
    static float agent_speed;                   /**< Distance per time unit. */
    static unsigned int agent_planning_window;  /**< Steps. 540 ~= 1 orbit. */
    static unsigned int agent_replanning_window;/**< Steps. 540 ~= 1 orbit. */
    static float activity_size;                 /**< Size of a single agent msg. */
    static AgentMotionType motion_model;        /**< Type of trajectory and motion model. */
    static TimeValueType time_type;             /**< Type of units in time magnitudes. */

    /* Orbital parameters: */
    static float orbp_sma_min;                  /**< Min. semi-major axis (in meters). */
    static float orbp_sma_max;                  /**< Max. semi-major axis (in meters). */
    static float orbp_ecc_max;                  /**< Max. eccentricity. */
    static float orbp_inc_min;                  /**< Min. inclination (in degrees). */
    static float orbp_inc_max;                  /**< Max. inclination (in degrees). */
    static float orbp_argp_min;                 /**< Min. argument of the perigee (in degrees). */
    static float orbp_argp_max;                 /**< Max. argument of the perigee (in degrees). */
    static float orbp_raan_min;                 /**< Min. Right Ascension of the Ascending node (in deg). */
    static float orbp_raan_max;                 /**< Max. Right Ascension of the Ascending node (in deg). */
    static float orbp_init_ma_max;              /**< Max. mean anomaly (in deg). */
    static float orbp_init_ma_min;              /**< Min. mean anomaly (in deg). */

    /* Resource consumptions and capacities: */
    /* -- Energy: */
    static float agent_energy_generation_rate;
    static float instrument_energy_min;
    static float instrument_energy_max;
    static float link_tx_energy_rate;
    static float link_rx_energy_rate;
    /* -- Storage: */
    static float instrument_storage_min;
    static float instrument_storage_max;

    /* Format and colors: */
    static sf::Font fnt_monospace;
    static sf::Font fnt_normal;
    static sf::Font fnt_awesome;
    static sf::Color color_orange;
    static sf::Color color_dark_green;
    static sf::Color color_link_los;
    static sf::Color color_link_connected;
    static sf::Color color_link_sending;
    static ColorGradient color_gradient_rgb;
    static ColorGradient color_gradient_rbg;
    static ColorGradient color_gradient_krbg;
    static ColorGradient color_gradient_rainbow;
    static ColorGradient color_gradient_1;
    static ColorGradient color_gradient_blue;
    static unsigned int fnt_size;

    /* Activity management, scheduling configuration, and knowledge base parameters: */
    static unsigned int knowledge_base_size;    /**< Max. total number of activities that an agent can know. */
    static unsigned int max_tasks;              /**< Maximum number of tasks (i.e. chromosome length). */
    static float min_payoff;                    /**< Payoff threshold below which tasks will not be generated. */
    static double max_task_duration;            /**< Max. task duration (in time units). */
    static double activity_confirm_window;      /**< Length of the confirmation window (in time units). */
    static float confidence_mod_exp;            /**< Exponent of the confidence modifier. */
    static float utility_floor;                 /**< Minimum utility for confidence in priority. */
    static float utility_k;                     /**< Utility function (logitic sigmoid func): steepness. */
    static float utility_unknown;               /**< Utility obtained for payoff computed without other activities. */
    static float utility_weight;                /**< Priority function: weight for utility. */
    static float decay_weight;                  /**< Priority function: weight for decay. */

    /* Genetic Algorithm configuration: */
    static unsigned int ga_generations;         /**< Max. absolute number of iterations. */
    static unsigned int ga_timeout;             /**< Max. number of iterations without improvement. */
    static float ga_min_improvement_rate;       /**< Min. fitness/iteration rate to complete. */
    static Aggregate ga_payoff_aggregate;       /**< Aggregation method for activity payoff values. */
    static unsigned int ga_population_size;     /**< Number of individuals in the population. */
    static unsigned int ga_crossover_points;    /**< Points of chromosome crossover. */
    static unsigned int ga_tournament_k;        /**< Parameter K in tournament selection operator. */
    static float ga_mutation_rate;              /**< Mutation probability. */
    static GASCrossoverOp ga_crossover_op;      /**< Crossover operator. */
    static GASSelectionOp ga_parentsel_op;      /**< Parent selection operator. */
    static GASSelectionOp ga_environsel_op;     /**< Environment/combination operator. */
    static float ga_payoff_k;                   /**< P.O. multiplier for previous activities. */
    static float ga_confidence_th;              /**< Min. confidence to use the P.O. multiplier. */

    /* Global values: */
    static std::string root_path;   /**< Root path of the project. */
    static std::string data_path;   /**< Path were simulation results will be saved to.*/
    static SandboxMode mode;        /**< The mode of this sandbox. */

    /*******************************************************************************************//**
     *  Loads command arguments from console and parses them.
     *  @param argc     Argument count.
     *  @param argv     Argument list.
     **********************************************************************************************/
    static void loadCmdArgs(int argc, char** argv);

private:
    /*******************************************************************************************//**
     *  Parses configuration parameter named with pname from YAML node n, and stores it in val.
     *  @param  pname   The name of the YAML node.
     *  @param  n       The YAML node object where the actual node must be looked for.
     *  @param  val     The variable where the value will be stored (if found).
     **********************************************************************************************/
    template <class T>
    static bool getConfigParam(std::string pname, const YAML::Node& n, T& val);

    /*******************************************************************************************//**
     *  Parses configuration parameter named with pname from YAML node n, and stores it in val. This
     *  version shall be used for nodes the content of which is a sequence of number (i.e. an array)
     *  with, at least, two elements. These two values will be used as boundaries to generate the
     *  random value that will be stored in val.
     *  @param  pname   The name of the YAML node that contains a sequence.
     *  @param  n       The YAML node object where the actual node must be looked for.
     *  @param  val     The variable where the value will be stored (if found).
     **********************************************************************************************/
    template <class T>
    static bool getConfigRandomParam(std::string pname, const YAML::Node& n, T& val);

    /*******************************************************************************************//**
     *  Parses configuration parameter named with pname from YAML node n, and stores it in val. This
     *  version expects the node to contain a sequence (i.e. array of numbers). The first and second
     *  element in that sequence will be stored in val_a and val_b, respectively.
     *  @param  pname   The name of the YAML node that contains a sequence.
     *  @param  n       The YAML node object where the actual node must be looked for.
     *  @param  val_a   The variable where the first value in the sequence will be stored.
     *  @param  val_b   The variable where the second value in the sequence will be stored.
     **********************************************************************************************/
    template <class T>
    static bool getConfigParam(std::string pname, const YAML::Node& n, T& val_a, T& val_b);

    /*******************************************************************************************//**
     *  Helper function to use logger functions (error formated).
     **********************************************************************************************/
    static void logProxyErr(std::string s);

    /*******************************************************************************************//**
     *  Helper function to use logger functions (warning formated).
     **********************************************************************************************/
    static void logProxyWarn(std::string s);

    /*******************************************************************************************//**
     *  Helper function to use logger functions (debug formated).
     **********************************************************************************************/
    static void logProxyDbg(std::string s);

    /*******************************************************************************************//**
     *  Helper function to use random functions (integer version).
     **********************************************************************************************/
    static int randomProxy(int a, int b);

    /*******************************************************************************************//**
     *  Helper function to use random functions (float version).
     **********************************************************************************************/
    static float randomProxy(float a, float b);
};

template <class T>
bool Config::getConfigParam(std::string pname, const YAML::Node& n, T& val)
{
    std::stringstream ss;
    if(n[pname].IsDefined()) {
        val = n[pname].as<T>();
        ss << " -- Config. parameter \'" << pname << "\' is set to: " << std::boolalpha << val << ".\n";
        logProxyDbg(ss.str());
        return true;
    } else {
        ss << " -- Config. parameter \'" << pname << "\' is not defined. Default value: " << std::boolalpha << val << ".\n";
        logProxyWarn(ss.str());
        return false;
    }
}

template <class T>
bool Config::getConfigParam(std::string pname, const YAML::Node& n, T& val_a, T& val_b)
{
    std::stringstream ss;
    if(n[pname].IsDefined() && n[pname].IsSequence() && n[pname].size() >= 2) {
        val_a = n[pname][0].as<T>();
        val_b = n[pname][1].as<T>();
        ss << " -- Config. parameter \'" << pname << "\' is set to: [" << val_a << ", " << val_b << "].\n";
        logProxyDbg(ss.str());
        return true;
    } else if(n[pname].IsDefined() && n[pname].IsScalar()) {
        val_a = n[pname].as<T>();
        val_b = n[pname].as<T>();
        ss << " -- Config. parameter \'" << pname << "\' is set to: [" << val_a << ", " << val_b << "].\n";
        logProxyDbg(ss.str());
        return true;
    } else {
        ss << " -- Config. parameter \'" << pname << "\' is not defined. Default values: [" << val_a << ", " << val_b << "].\n";
        logProxyWarn(ss.str());
        return false;
    }
}

template <class T>
bool Config::getConfigRandomParam(std::string pname, const YAML::Node& n, T& val)
{
    std::stringstream ss;
    if(n[pname].IsDefined() && n[pname].IsSequence() && n[pname].size() >= 2) {
        val = randomProxy(n[pname][0].as<T>(), n[pname][1].as<T>());
        ss << "Config. parameter \'" << pname << "\' is randomly set to: " << val << ".\n";
        logProxyDbg(ss.str());
        return true;
    } else {
        ss << "Config. parameter \'" << pname << "\' is not defined. Default value: " << val << ".\n";
        logProxyWarn(ss.str());
        return false;
    }
}


#endif /* CONFIG_HPP */
