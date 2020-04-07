/***********************************************************************************************//**
 *  Global virtual time.
 *  @class      VirtualTime
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-19
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "VirtualTime.hpp"

CREATE_LOGGER(VirtualTime)

double VirtualTime::m_vtime;
bool VirtualTime::m_initialized = false;

void VirtualTime::step(void)
{
    m_vtime += Config::time_step;
}

bool VirtualTime::finished(void)
{
    return m_vtime >= Config::start_epoch + Config::duration;
}

void VirtualTime::doInit(double t)
{
    m_vtime = t;
    m_initialized = true;
}

std::string VirtualTime::toString(double t, bool is_absolute_time, bool simplified)
{
    std::stringstream ss;
    std::string retval;

    if(t == -1.0) {
        t = m_vtime;
        is_absolute_time = true;
    }

    if(Config::time_type == TimeValueType::JULIAN_DAYS) {
        if(is_absolute_time) {
            if(!m_initialized) {
                if(simplified) {
                    ss << "?d??:??:??";
                } else {
                    ss << "\'?d??:??:??.???\'";
                }
                return ss.str();
            } else {
                t -= Config::start_epoch;
            }
        }
        double sec = std::fmod(t * 60.0 * 60.0 * 24.0, 60.0);
        int min  = (int)(t * 60 * 24) % 60;
        int hour = (int)(t * 24) % 24;
        int days = (int)t;
        if(60.0 - sec < 0.001) {
            /* Just for visalisation purposes. */
            sec = 0;
            min++;
            if(min >= 60) {
                min = min % 60;
                hour++;
                if(hour >= 24) {
                    hour = hour % 24;
                    days++;
                }
            }
        }
        int precision = 3;
        int numberwidth = 6;
        int sec_integer = ((int)sec) % 60;
        if(simplified) {
            precision = 0;
            numberwidth = 2;
            sec = sec_integer;
        } else {
            ss << "\'";
        }
        ss << days << "d"
            << std::setw(2) << std::setfill('0') << hour << ":"
            << std::setw(2) << std::setfill('0') << min  << ":"
            << std::fixed << std::setprecision(precision) << std::setw(numberwidth) << std::setfill('0') << sec;
        if(!simplified) {
            ss << "\'";
        }
    } else {
        ss << std::fixed << std::setprecision(3) << m_vtime;
    }
    retval = ss.str();
    return retval;
}

double VirtualTime::toVirtual(double t, TimeValueType type)
{
    if(type == Config::time_type ||
        Config::time_type == TimeValueType::ARBITRARY ||
        type == TimeValueType::ARBITRARY
    ) {
        /* Nothing to convert. */
        return t;
    } else {
        if(type == TimeValueType::SECONDS && Config::time_type == TimeValueType::JULIAN_DAYS) {
            return t / (24.0 * 3600.0);
        } else if(type == TimeValueType::JULIAN_DAYS && Config::time_type == TimeValueType::SECONDS) {
            return t * 24.0 * 3600.0;
        }
    }
    throw std::runtime_error("Error converting to virtual time");
}
