/***********************************************************************************************//**
 *  Converts a finite range of values to color.
 *  @class      ColorGradient
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-abr-05
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef COLOR_GRADIENT_HPP
#define COLOR_GRADIENT_HPP

#include "prot.hpp"

class ColorGradient
{
public:
    ColorGradient(void);
    ColorGradient(std::map<float, sf::Color> colors);
    sf::Color getColorAt(float v);
    void setGradient(std::map<float, sf::Color> cg) { m_color_steps = cg; }

private:
    std::map<float, sf::Color> m_color_steps;
};

#endif /* COLOR_GRADIENT_HPP */
