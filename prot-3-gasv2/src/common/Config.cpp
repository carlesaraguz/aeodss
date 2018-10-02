/***********************************************************************************************//**
 *  Program configuration values.
 *  @class      Config
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Config.hpp"
#include "ColorGradient.hpp"

sf::Font Config::fnt_monospace;
sf::Font Config::fnt_normal;
sf::Font Config::fnt_awesome;
sf::Color Config::color_orange;
sf::Color Config::color_dark_green;
ColorGradient Config::color_gradient_rgb;
ColorGradient Config::color_gradient_rbg;
ColorGradient Config::color_gradient_krbg;
ColorGradient Config::color_gradient_1;
ColorGradient Config::color_gradient_blue;
unsigned int Config::max_task_duration;
unsigned int Config::ga_crossover_points = 5;
std::string Config::root_path;
