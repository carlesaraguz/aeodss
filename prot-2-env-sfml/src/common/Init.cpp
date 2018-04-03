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
    Config::fnt_monospace.loadFromFile("/usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf");
    Config::fnt_normal.loadFromFile("/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-L.ttf");
    Config::fnt_awesome.loadFromFile("/usr/share/fonts/opentype/font-awesome/FontAwesome.otf");
    Config::color_orange = sf::Color(255, 153, 102);
    Config::color_dark_green = sf::Color(35, 94, 92);

    Random::doInit();
}
