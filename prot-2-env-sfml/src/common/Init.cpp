/***********************************************************************************************//**
 *  Program initialization and setup class.
 *  @class      Init
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-feb-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Init.hpp"

void Init::doInit(void)
{
    Random::doInit();

    Config::fnt_monospace.loadFromFile("/usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf");
    Config::fnt_normal.loadFromFile("/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-L.ttf");
    Config::fnt_awesome.loadFromFile("/usr/share/fonts/opentype/font-awesome/FontAwesome.otf");
    Config::color_orange = sf::Color(255, 153, 102);
    Config::color_dark_green = sf::Color(35, 94, 92);

    std::map<float, sf::Color> gradient;
    gradient[0.f]  = sf::Color::Red;
    gradient[0.5f] = sf::Color::Green;
    gradient[1.f]  = sf::Color::Blue;
    Config::color_gradient_rgb.setGradient(gradient);
    gradient.clear();
    gradient[0.f]  = sf::Color(173,   0,   0, 255);     /* Dark red.    */
    gradient[0.5f] = sf::Color(255, 165,   0, 255);     /* Orange.      */
    gradient[1.f]  = sf::Color(  0, 123, 255, 255);     /* Sky blue.    */
    Config::color_gradient_1.setGradient(gradient);
    gradient.clear();
    gradient[0.f]  = sf::Color( 13,  50,  99, 255);     /* Dark blue.   */
    gradient[0.5f] = sf::Color( 64, 143, 247, 255);     /* Light blue.  */
    gradient[1.f]  = sf::Color(219, 234, 255, 255);     /* Pale blue.   */
    Config::color_gradient_blue.setGradient(gradient);
    gradient.clear();

    if(Config::ga_crossover_points > Config::max_tasks - 1) {
        Config::ga_crossover_points = Random::getUi(1, Config::max_tasks - 1);
        std::cerr << "GA Scheduler: crossover points changed to " << Config::ga_crossover_points << ".\n";
    }
}
