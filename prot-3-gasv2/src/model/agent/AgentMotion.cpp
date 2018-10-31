/***********************************************************************************************//**
 *  The motion model of an agent in the world.
 *  @class      AgentMotion
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-06
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "AgentMotion.hpp"

CREATE_LOGGER(AgentMotion)

AgentMotion::AgentMotion(Agent* aptr, sf::Vector3f init_pos, sf::Vector3f init_vel)
    : m_agent(aptr)
    , m_world_h(Config::world_height)
    , m_world_w(Config::world_width)
{
    switch(Config::motion_model) {
        case AgentMotionType::LINEAR_BOUNCE:
            {
                if(init_pos == sf::Vector3f(-1.f, -1.f, -1.f)) {
                    init_pos = sf::Vector3f(
                        Random::getUf(0.f, m_world_w),
                        Random::getUf(0.f, m_world_h),
                        0.f
                    );
                }
                if(init_vel == sf::Vector3f(0.f, 0.f, 0.f)) {
                    float theta = Random::getUf(0.f, 360.f);
                    init_vel = {
                        std::cos(theta),
                        std::sin(theta),
                        0.f
                    };
                    init_vel *= Config::agent_speed;
                }
                m_position.push_back(init_pos);
                m_velocity.push_back(init_vel);
                m_orbital_state.push_back({});
            }
            break;
        case AgentMotionType::LINEAR_INFINITE:
            Log::err << "[ TODO ] Constructing AgentMotion with an unsupported motion model: LINEAR_INFINITE.\n";
            throw std::runtime_error("Constructing AgentMotion with unsupported motion model LINEAR_INFINITE.");
            break;
        case AgentMotionType::SINUSOIDAL:
            Log::err << "[ TODO ] Constructing AgentMotion with an unsupported motion model: SINUSOIDAL.\n";
            throw std::runtime_error("Constructing AgentMotion with unsupported motion model SINUSOIDAL.");
            break;
        case AgentMotionType::ORBITAL:
            Log::err << "[ TODO ] Constructing AgentMotion with an unsupported motion model: ORBITAL.\n";
            throw std::runtime_error("Constructing AgentMotion with unsupported motion model ORBITAL.");
            break;
        }
}

AgentMotion::AgentMotion(Agent* aptr, double init_mean_an, OrbitalParams pars)
    : m_agent(aptr)
    , m_world_h(Config::world_height)
    , m_world_w(Config::world_width)
    , m_orb_params(pars)
{
    switch(Config::motion_model) {
        case AgentMotionType::LINEAR_BOUNCE:
            Log::err << "[" << m_agent->getId() << "] Unimplemented motion model LINEAR_BOUNCE.\n";
            throw std::runtime_error("Unimplemented motion model.");
            break;
        case AgentMotionType::LINEAR_INFINITE:
            Log::err << "[" << m_agent->getId() << "] Unimplemented motion model LINEAR_INFINITE.\n";
            throw std::runtime_error("Unimplemented motion model.");
            break;
        case AgentMotionType::SINUSOIDAL:
            Log::err << "[" << m_agent->getId() << "] Unimplemented motion model SINUSOIDAL.\n";
            throw std::runtime_error("Unimplemented motion model.");
            break;
        case AgentMotionType::ORBITAL:
            if(m_orb_params.sma == -1.0) {
                /* Earth semi-major axis of WGS84 */
                m_orb_params.sma  = (double)Random::getUf(500e3f, 900e3f) + Config::earth_wgs84_a;
                m_orb_params.ecc  = (double)Random::getUf(0.f, 0.02f);
                m_orb_params.inc  = (double)Random::getUf(70.f, 90.f);
                m_orb_params.argp = (double)Random::getUf(0.f, 360.f);
                m_orb_params.raan = (double)Random::getUf(0.f, 360.f);
                // Log::dbg << "Randomly generated orbital parameters:\n";
                // Log::dbg << " - Semi-major axis: " << m_orb_params.sma << " meters (R+ " << m_orb_params.sma - Config::earth_wgs84_a << " meters).\n";
                // Log::dbg << " - Eccentricity: " << m_orb_params.ecc << ".\n";
                // Log::dbg << " - Inclination: " << m_orb_params.inc << " deg.\n";
                // Log::dbg << " - Arg. of perigee: " << m_orb_params.argp << " deg.\n";
                // Log::dbg << " - RAAN: " << m_orb_params.raan << " deg.\n";
            }

            m_orb_params.mean_motion = std::sqrt(Config::earth_mu / std::pow(m_orb_params.sma, 3)); /* In radians/sec. */
            // Log::dbg << "Mean motion: " << m_orb_params.mean_motion << " rad/s.\n";

            OrbitalState os;
            os.mean_anomaly = init_mean_an;
            os.true_anomaly = transfMeanToTrue(os.mean_anomaly);
            os.ecc_anomaly  = transfMeanToEccentric(os.mean_anomaly);
            os.radius       = getRadiusLength(os.true_anomaly);
            m_orbital_state.push_back(os);

            /* Compute and store new positions and velocities: */
            m_position.push_back(getPositionFromOrbital(os));
            m_velocity.push_back(getVelocityFromOrbital(os));

            // Log::dbg << "Initial position: "
            //     << "p(" << m_position.back().x << ", " << m_position.back().y << ", " << m_position.back().z << ") "
            //     << "v(" << m_velocity.back().x << ", " << m_velocity.back().y << ", " << m_velocity.back().z << ") "
            //     << "ma = " << os.mean_anomaly << " rad.\n";
            break;
        }
}

void AgentMotion::step(void)
{
    if(m_position.size() == 1) {
        propagate(2);
    }

    if(m_position.size() > 1) {
        m_position.erase(m_position.begin());
        m_velocity.erase(m_velocity.begin());
    } else {
        Log::warn << "[" << m_agent->getId() << "] Agent motion failure (" << m_position.size() << ").\n";
    }
}

void AgentMotion::clearPropagation(void)
{
    std::vector<sf::Vector3f> p(1, m_position.front());
    std::vector<sf::Vector3f> v(1, m_velocity.front());
    std::vector<OrbitalState> o(1, m_orbital_state.front());
    m_position.swap(p);
    m_velocity.swap(v);
    m_orbital_state.swap(o);
}

std::vector<sf::Vector3f> AgentMotion::propagate(unsigned int nsteps)
{
    if(nsteps > m_position.size()) {
        unsigned int count = nsteps - m_position.size();
        switch(Config::motion_model) {
            case AgentMotionType::LINEAR_BOUNCE:
                {
                    sf::Vector3f p0 = m_position.back();
                    sf::Vector3f v0 = m_velocity.back();
                    sf::Vector3f p, v;
                    for(unsigned int i = 0; i < count; i++) {
                        sf::Vector3f dp = v0 * Config::time_step;
                        move(p0, v0, dp, p, v);
                        m_position.push_back(sf::Vector3f(p.x, p.y, 0));
                        m_velocity.push_back(sf::Vector3f(v.x, v.y, 0));
                        p0 = p;
                        v0 = v;
                    }
                }
                break;
            case AgentMotionType::ORBITAL:
                {
                    OrbitalState os0 = m_orbital_state.back();
                    OrbitalState os;
                    double dt  = Config::time_step * 3600.0 * 24.0;     /* Julian days to seconds.*/
                    for(unsigned int i = 0; i < count; i++) {
                        double dma = m_orb_params.mean_motion * dt;     /* In radians. */
                        os.mean_anomaly = std::fmod(os0.mean_anomaly + dma, 2 * Config::pi);
                        os.true_anomaly = transfMeanToTrue(os.mean_anomaly);
                        os.ecc_anomaly  = transfMeanToEccentric(os.mean_anomaly);
                        os.radius       = getRadiusLength(os.true_anomaly);
                        m_orbital_state.push_back(os);

                        /* Compute and store new positions and velocities: */
                        m_position.push_back(getPositionFromOrbital(os));
                        m_velocity.push_back(getVelocityFromOrbital(os));

                        os0 = os;

                        // Log::dbg << "Propagation " << i << ": "
                        //     << "p(" << m_position.back().x << ", " << m_position.back().y << ", " << m_position.back().z << ") "
                        //     << "v(" << m_velocity.back().x << ", " << m_velocity.back().y << ", " << m_velocity.back().z << ")\n";
                    }
                }
                break;
            default:
                /* Does nothing. */
                break;
        }
        return m_position;
    } else {
        return std::vector<sf::Vector3f>(m_position.begin(), m_position.begin() + nsteps);
    }
}

void AgentMotion::move(sf::Vector3f p0, sf::Vector3f v0, sf::Vector3f dp, sf::Vector3f& p, sf::Vector3f& v) const
{
    /*  Bounce flags:
     *  bx0 -> bounces at x=0.
     *  bx1 -> bounces at x=w.
     *  by0 -> bounces at y=0.
     *  by1 -> bounces at y=h.
     **/
    bool bx0 = false, bx1 = false, by0 = false, by1 = false;
    float newx = 0.f, newy = 0.f, displacement_ratio;

    if(inBounds(sf::Vector2f((p0 + dp).x, (p0 + dp).y))) {
        p = p0 + dp;
        v = v0;
    } else {
        /* Determine bounces: */
        sf::Vector3f ptmp = p0 + dp;
        if(ptmp.x < 0.f) {
            bx0 = true;
        }
        if(ptmp.x > m_world_w) {
            bx1 = true;
        }
        if(ptmp.y < 0.f) {
            by0 = true;
        }
        if(ptmp.y > m_world_h) {
            by1 = true;
        }
        if(bx0 && by0) {
            if(-ptmp.x >= -ptmp.y) {
                by0 = false;
            } else {
                bx0 = false;
            }
        } else if(by0 && bx1) {
            if(-ptmp.y >= ptmp.x - m_world_w) {
                bx1 = false;
            } else {
                by0 = false;
            }
        } else if(bx1 && by1) {
            if(ptmp.x - m_world_w >= ptmp.y - m_world_h) {
                by1 = false;
            } else {
                bx1 = false;
            }
        } else if(by1 && bx0) {
            if(ptmp.y - m_world_h >= -ptmp.x) {
                bx0 = false;
            } else {
                by1 = false;
            }
        }
        if(bx0) {
            /* Bounce at x=0: */
            newx = 0.f;
            displacement_ratio = std::fabs(p0.x / dp.x);
            newy = p0.y + displacement_ratio * dp.y;
            dp.x = -dp.x - p0.x;
            dp.y *= (1.f - displacement_ratio);
            v0.x = -v0.x;
        } else if(by0) {
            /* Bounce at y=0: */
            newy = 0.f;
            displacement_ratio = std::fabs(p0.y / dp.y);
            newx = p0.x + displacement_ratio * dp.x;
            dp.y = -dp.y - p0.y;
            dp.x *= (1.f - displacement_ratio);
            v0.y = -v0.y;
        } else if(bx1) {
            /* Bounce at x=w: */
            newx = m_world_w;
            displacement_ratio = std::fabs((m_world_w - p0.x) / dp.x);
            newy = p0.y + displacement_ratio * dp.y;
            dp.x = -dp.x - (m_world_w - p0.x);
            dp.y *= (1.f - displacement_ratio);
            v0.x = -v0.x;
        } else if(by1) {
            /* Bounce at y=h: */
            newy = m_world_h;
            displacement_ratio = std::fabs((m_world_h - p0.y) / dp.y);
            newx = p0.x + displacement_ratio * dp.x;
            dp.y = -dp.y - (m_world_h - p0.y);
            dp.x *= (1.f - displacement_ratio);
            v0.y = -v0.y;
        }
        p0.x = newx;
        p0.y = newy;
        move(p0, v0, dp, p, v);
    }
}

bool AgentMotion::inBounds(const sf::Vector2f& p) const
{
    return (p.x >= 0.f && p.x <= m_world_w) && (p.y >= 0.f && p.y <= m_world_h);
}

sf::Vector3f AgentMotion::getPositionFromOrbital(OrbitalState os) const
{
    return CoordinateSystemUtils::fromOrbitaltoECI(
        os.radius,
        os.true_anomaly,
        m_orb_params.raan,
        m_orb_params.argp,
        m_orb_params.inc
    );
}

sf::Vector3f AgentMotion::getVelocityFromOrbital(OrbitalState os) const
{
    float p_x, p_y, p_z;
    float q_x, q_y, q_z;
    double argp = MathUtils::degToRad(m_orb_params.argp);
    double raan = MathUtils::degToRad(m_orb_params.raan);
    double inc  = MathUtils::degToRad(m_orb_params.inc);

    p_x  = std::cos(argp) * std::cos(raan);
    p_x -= std::sin(raan) * std::sin(argp) * std::cos(inc);
    p_y  = std::cos(argp) * std::sin(raan);
    p_y -= std::cos(raan) * std::sin(argp) * std::cos(inc);
    p_z  = std::sin(argp) * std::sin(inc);

    q_x  = -std::sin(argp) * std::cos(raan);
    q_x -=  std::sin(raan) * std::cos(argp) * std::cos(inc);
    q_y  = -std::sin(argp) * std::sin(raan);
    q_y -=  std::cos(raan) * std::cos(argp) * std::cos(inc);
    q_z  =  std::cos(argp) * std::sin(inc);

    float e_prime = std::sqrt(Config::earth_mu / (m_orb_params.sma * getRadiusLength(os.true_anomaly)));

    float s_min_axis = m_orb_params.sma * std::sqrt(1 - m_orb_params.ecc * m_orb_params.ecc);

    float v_x_prime = - m_orb_params.sma * e_prime * std::cos(os.ecc_anomaly);
    float v_y_prime = s_min_axis * e_prime * std::cos(os.ecc_anomaly);

    float v_x = v_x_prime * p_x + v_y_prime * q_x;
    float v_y = v_x_prime * p_y + v_y_prime * q_y;
    float v_z = v_x_prime * p_z + v_y_prime * q_z;

    return sf::Vector3f(v_x, v_y, v_z);
}

sf::Vector2f AgentMotion::getProjection2D(void) const
{
    sf::Vector2f retvec;
    switch(Config::motion_model) {
        case AgentMotionType::LINEAR_BOUNCE:
        case AgentMotionType::LINEAR_INFINITE:
        case AgentMotionType::SINUSOIDAL:
            retvec = sf::Vector2f(m_position.front().x, m_position.front().y);
            break;
        case AgentMotionType::ORBITAL:
            retvec = getProjection2D(m_position.front(), VirtualTime::now());
            // Log::dbg << "2D projection: p(" << retvec.x << ", " << retvec.y << ")\n";
            break;
    }
    return retvec;
}

sf::Vector2f AgentMotion::getProjection2D(sf::Vector3f p, double t)
{
    sf::Vector3f ecef_coord = CoordinateSystemUtils::fromECItoECEF(p, t);
    sf::Vector3f geo_coord  = CoordinateSystemUtils::fromECEFtoGeographic(ecef_coord);
    return sf::Vector2f(
        (float)(Config::world_width * (geo_coord.y)  / 360.f) + (Config::world_width / 2.f),
        (float)(Config::world_height * (geo_coord.x) / 180.f) + (Config::world_height / 2.f)
    );
}

sf::Vector2f AgentMotion::getDirection2D(void)
{
    sf::Vector2f retvec;
    switch(Config::motion_model) {
        case AgentMotionType::LINEAR_BOUNCE:
        case AgentMotionType::LINEAR_INFINITE:
        case AgentMotionType::SINUSOIDAL:
            retvec = sf::Vector2f(m_velocity.front().x, m_velocity.front().y);
            break;
        case AgentMotionType::ORBITAL:
            {
                propagate(2);   /* Ensure that there are at least two positions computed: */
                sf::Vector2f curr = AgentMotion::getProjection2D(m_position[0], VirtualTime::now());
                sf::Vector2f next = AgentMotion::getProjection2D(m_position[1], VirtualTime::now() + Config::time_step);

                sf::Vector2f vel = sf::Vector2f(next.x - curr.x, next.y - curr.y) / Config::time_step;
                sf::Vector2f norm_vel = vel / std::sqrt(vel.x * vel.x + vel.y * vel.y);

                return norm_vel;
            }
            break;
    }
    return sf::Vector2f(); /* Idem as getProjection2D */
}

double AgentMotion::getRadiusLength(double true_an) const
{
    double num = m_orb_params.sma * (1 - m_orb_params.ecc * m_orb_params.ecc);
    double den = 1 + m_orb_params.ecc * std::cos(true_an);

    return num / den;
}

double AgentMotion::transfEccentricToTrue(double ecc_anomaly) const
{
    double y = std::sqrt(1 + m_orb_params.ecc) * std::sin(ecc_anomaly / 2);
    double x = std::sqrt(1 - m_orb_params.ecc) * std::cos(ecc_anomaly / 2);
    return 2 * std::atan2(y, x);
}


double AgentMotion::transfTrueToEccentric(double true_anomaly) const
{
    double num = std::sqrt(1 - m_orb_params.ecc * m_orb_params.ecc) * std::sin(true_anomaly);
    double den = m_orb_params.ecc + std::cos(true_anomaly);

    return std::atan2(num, den);
}


double AgentMotion::transfMeanToEccentric(double mean_anomaly) const
{
    /*  NOTE: Newton-Raphson numerical method (see below):
     *      Formula:
     *          M = E - ecc * std::sin(E)
     *      We are not able to isolate E so we will use numeric methods as we said previously.
     *          f(E) = E - ecc * sin (E) - M
     *          f'(E) = 1 + ecc * std::cos(E)
     *      Starting Value = mean_anomaly
     *          x_i+1 = x_0 - f(x_i) / f'(x_i)
     **/
     double tmp = mean_anomaly;
     double num;
     double den;

     for(int i = 0; i < 10; i++) {
         num = tmp - m_orb_params.ecc * std::sin(tmp) - mean_anomaly; /* f(E) */
         den = 1 + m_orb_params.ecc * std::cos(tmp);
         tmp = tmp - num / den;
     }
     return tmp;
}

double AgentMotion::transfMeanToTrue(double mean_anomaly) const
{
    return transfTrueToEccentric(transfMeanToEccentric(mean_anomaly));
}
