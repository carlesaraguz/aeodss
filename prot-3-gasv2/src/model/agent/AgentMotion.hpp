/***********************************************************************************************//**
 *  The motion model of an agent in the world.
 *  @class      AgentMotion
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *              Marc Closa (MC), marc.closa.tarres@alu-etsetb.upc.edu
 *  @date       2018-nov-15
 *  @version    0.2
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef AGENT_MOTION_HPP
#define AGENT_MOTION_HPP

#include "prot.hpp"
#include "TimeStep.hpp"
#include "Random.hpp"
#include "Config.hpp"
#include "CoordinateSystemUtils.hpp"
#include "VirtualTime.hpp"
#include "MathUtils.hpp"

#include <math.h>

class Agent;

struct OrbitalParams {
    double sma;             /**< Semi-major axis (in meters). */
    double ecc;             /**< Eccentricity (unit-less). */
    double inc;             /**< Inclination (degrees). */
    double argp;            /**< Argument of the perigee (degrees). */
    double raan;            /**< Right-ascension of the Ascending node (degrees). */
    double mean_motion;     /**< Mean motion (rad/s). */
};

/*******************************************************************************************//**
 *  It provides the core functions for agents' propagation. The class supports two different
 *  motion types (According to the Config::motion_model):
 *  - Config::motion_model = LINEAR_BOUNCE: Computes and represents a 2D finite world where
 *    agents propagate following a line and when they reach the bounds, rebound and go back.
 *  - Config::motion_model = ORBITAL: It is an extension into 3D where the agents describe
 *    an elliptic orbit following the Keplerian Orbit principles. Also is able to project
 *    into 2D current position and velocity (Equirectangular projection is used to project
 *    agents' position).
 **********************************************************************************************/
class AgentMotion : public TimeStep
{
public:
    /*******************************************************************************************//**
     *  Specific constructor of AgentMotion implemented when LINEAR_BOUNCE motion type is chosen.
     *  Position and velocity are randomly generated using an uniform pdf value generator.
     **********************************************************************************************/
    AgentMotion(Agent* aptr, sf::Vector3f init_pos = {-1.f, -1.f, -1.f}, sf::Vector3f init_vel = {0.f, 0.f, 0.f});

    /*******************************************************************************************//**
     *  Specific constructor of AgentMotion implemented when ORBITAL motion type is chosen.
     *  Orbital parameters are initialized randomly (with some restrictions taking into account
     *  the agent follows a LEO orbit), then computes all anomalies (True, Mean and Eccentric) and
     *  finally position (in ECI Coordinates) and instantanious velocity are computed.
     **********************************************************************************************/
    AgentMotion(Agent* aptr, double init_mean_an, OrbitalParams orb_pars = {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0});

    /*******************************************************************************************//**
     *  Computes the new position and velocity of one single step in any type of motion and updates
     *  the position and velocity vectors.
     **********************************************************************************************/
    void step(void) override;

    /*******************************************************************************************//**
     *  Generates a vector of nsteps positions. If there is no position in the vector, propagates
     *  nsteps. If there is alredy a certain number of positions, it propagates nsteps - #positions.
     *  It returns a vector of 3D positions of nsteps length.
     *  If motion model is:
     *      - 2D MOTION MODEL (LINEAR_BOUNCE, LINEAR_INFINITE, SINUSOIDAL):
     *        It returns a vector of 3D positions where X,Y are the current positions in the
     *        enviroment whereas Z always is equal to 0.
     *      - 3D MOTION MODEL (ORBITAL):
     *        It returns a vector of 3D positions in ECI Coordinates.
     **********************************************************************************************/
    std::vector<sf::Vector3f> propagate(unsigned int nsteps);

    /*******************************************************************************************//**
     *  Clears all position buffer and leaves the last or current position. The size of the
     *  position vectors is modified into 1. Occurs the same in the velocity vector.
     **********************************************************************************************/
    void clearPropagation(void);

    /*******************************************************************************************//**
     *  Getter of the current position. Acceses to the last position of the position vector and returns
     *  the 3D vector of position in that position.
     **********************************************************************************************/
    sf::Vector3f getPosition(void) const { return m_position.front(); }

    /*******************************************************************************************//**
     *  Getter of the current velocity. Acceses to the last position of the velocity vector and returns
     *  the 3D vector of velocity in that position.
     **********************************************************************************************/
    sf::Vector3f getVelocity(void) const { return m_velocity.front(); }

    /*******************************************************************************************//**
     *  Returns the projection of current position according to the equirectangular projection.
     **********************************************************************************************/
    sf::Vector2f getProjection2D(void) const;

    /*******************************************************************************************//**
     *  Returns the equirectangular projection from a given ECI Coordinates in a determined instant
     *  in Julian Date.
     **********************************************************************************************/
    static sf::Vector2f getProjection2D(sf::Vector3f p, double t);

    /*******************************************************************************************//**
     *  Returns the normalized projection of velocity so as to determine the current direction.
     **********************************************************************************************/
    sf::Vector2f getDirection2D(void);

    /*******************************************************************************************//**
     *  Getter for the semi-major axis.
     **********************************************************************************************/
    double getSMA(void) const { return m_orb_params.sma; }

    /*******************************************************************************************//**
     *  Computes the maximum altitude that this agent will ever get to. For 3-d orbital motion
     *  models this function computes the orbital radius at the apogee. In 2-d modes, this function
     *  always returns 0.
     *  @return     Orbital height (in meters) at apogee.
     **********************************************************************************************/
    double getMaxAltitude(void) const;

    /*******************************************************************************************//**
     *  Computes the minimum altitude that this agent will ever get to. For 3-d orbital motion
     *  models this function computes the orbital radius at the perigee. In 2-d modes, this function
     *  always returns 0.
     *  @return     Orbital height (in meters) at perigee.
     **********************************************************************************************/
    double getMinAltitude(void) const;

    /*******************************************************************************************//**
     *  Shows debug info for the agent motion model.
     **********************************************************************************************/
    void debug(void) const;

private:
    /* Motion arguments: */
    std::vector<sf::Vector3f> m_position;
    std::vector<sf::Vector3f> m_velocity;
    float m_world_h;
    float m_world_w;
    Agent* m_agent;

    /* Orbital attributes for AgentMotionType::ORBITAL: */
    struct OrbitalState {
        double mean_anomaly;
        double ecc_anomaly;
        double true_anomaly;
        double radius;
    };
    OrbitalParams m_orb_params;
    std::vector<OrbitalState> m_orbital_state;

    /*******************************************************************************************//**
     *  Computes the new position vector according to the movement type implemented.
     **********************************************************************************************/
    void move(sf::Vector3f p0, sf::Vector3f v0, sf::Vector3f dp, sf::Vector3f& p, sf::Vector3f& v) const;

    /*******************************************************************************************//**
     *  In LINEAR_BOUNCE, checks if the entity is inside the defined world. Returns true if is inside
     *  and false if it is outside. Uses the parameters m_world_h and m_world_w to check it.
     **********************************************************************************************/
    bool inBounds(const sf::Vector2f& p) const;

    /*******************************************************************************************//**
     *  Computes the new radius value according to m_position that has the ECI coordinates of the
     *  agent.
     **********************************************************************************************/
    double getRadiusLength(double true_an) const;

    /*******************************************************************************************//**
     *  Computes the instantanious velocity vectors (X, Y, Z) and returns it.
     *  The formulae used are taken from:
     *      https://space.stackexchange.com/q/14095/22532
     **********************************************************************************************/
    sf::Vector3f getVelocityFromOrbital(OrbitalState os) const;

    /*******************************************************************************************//**
     *  Computes and returns the agent position in ECI Coordinate system from a particular
     *  orbital state.
     **********************************************************************************************/
    sf::Vector3f getPositionFromOrbital(OrbitalState os) const;

    /*******************************************************************************************//**
     *  Transforms the eccentric anomaly to true anomaly.
     **********************************************************************************************/
    double transfEccentricToTrue(double ecc_anomaly) const;

    /*******************************************************************************************//**
     *  Transforms the true anomaly to eccentric anomaly.
     **********************************************************************************************/
    double transfTrueToEccentric(double true_anomaly) const;

    /*******************************************************************************************//**
     *  Transforms mean anomaly to eccentric anomaly.
     **********************************************************************************************/
    double transfMeanToEccentric(double mean_anomaly) const;

    /*******************************************************************************************//**
     *  Transforms mean anomaly to true anomaly.
     **********************************************************************************************/
    double transfMeanToTrue(double mean_anomaly) const;
};

#include "Agent.hpp"

#endif /* AGENT_MOTION_HPP */
