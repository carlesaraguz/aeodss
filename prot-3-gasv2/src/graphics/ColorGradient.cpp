/***********************************************************************************************//**
 *  Converts a finite range of values to color.
 *  @class      ColorGradient
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-abr-05
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "ColorGradient.hpp"

ColorGradient::ColorGradient(void)
{
    m_color_steps[0.f] = sf::Color::Black;
    m_color_steps[1.f] = sf::Color::Black;
}

ColorGradient::ColorGradient(std::map<float, sf::Color> colors)
    : m_color_steps(colors)
{
    if(m_color_steps.size() == 0) {
        m_color_steps[0.f] = sf::Color::Black;
        m_color_steps[1.f] = sf::Color::Black;
    }
    if(m_color_steps.find(0.f) == m_color_steps.end()) {
        m_color_steps[0.f] = m_color_steps.cbegin()->second;
    }
    if(m_color_steps.find(1.f) == m_color_steps.end()) {
        m_color_steps[1.f] = m_color_steps.crbegin()->second;
    }
}

sf::Color ColorGradient::getColorAt(float v)
{
    if(v >= 0.f && v <= 1.f) {
        if(v == 0.f || v == 1.f) {
            return m_color_steps[v];
        }
        sf::Color c1, c2;
        float v1 = 0.f, v2 = 1.f;
        for(auto& cs : m_color_steps) {
            if(v > cs.first) {
                v1 = cs.first;
                c1 = cs.second;
            } else if(v < cs.first) {
                v2 = cs.first;
                c2 = cs.second;
                break;
            } else if(v == cs.first) {
                return cs.second;
            }
        }
        float p = (v - v1) / (v2 - v1);
        sf::Color retcolor;
        retcolor.r = c2.r * p + c1.r * (1.f - p);
        retcolor.g = c2.g * p + c1.g * (1.f - p);
        retcolor.b = c2.b * p + c1.b * (1.f - p);
        retcolor.a = c2.a * p + c1.a * (1.f - p);
        return retcolor;
    } else {
        return sf::Color::Black;
    }
}
