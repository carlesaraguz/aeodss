/***********************************************************************************************//**
 *  Graphical representation of the environment model or state.
 *  @class      EnvModelView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef ENV_MODEL_VIEW_HPP
#define ENV_MODEL_VIEW_HPP

#include "prot.hpp"
#include "EnvModel.hpp"

class EnvModelView : public sf::Drawable
{
public:
    EnvModelView(int w, int h, sf::Color init_color = sf::Color::Black);
    EnvModelView(const EnvModel& e);
    EnvModelView(const EnvModel& e, sf::Color init_color);

    void display(const EnvModel& e, unsigned int layer = 0);
    int getHeight(void) const { return m_height; }
    int getWidth(void) const { return m_width; }

private:
    sf::Sprite m_sprite;
    sf::Texture m_texture;
    int m_width;
    int m_height;
    sf::VertexArray m_grid;

    void setColor(int x, int y, sf::Color c);
    void setColor(std::vector<std::tuple<int,int> > units, sf::Color c);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif /* ENV_MODEL_VIEW_HPP */
