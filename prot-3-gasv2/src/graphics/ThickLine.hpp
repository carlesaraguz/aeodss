/***********************************************************************************************//**
 *  A line with thickness.
 *  @class      ThickLine
 *  @authors    Tina @ https://github.com/SFML/SFML/wiki/Source:-Line-segment-with-thickness
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef THICK_LINE_HPP
#define THICK_LINE_HPP

#include "prot.hpp"
#include "MathUtils.hpp"

class ThickLine : public sf::Drawable, public sf::Transformable
{
public:
    ThickLine(const sf::Vector2f& p1 = {0.f, 0.f}, const sf::Vector2f& p2 = {0.f, 0.f});
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    void setPoints(const sf::Vector2f& p1, const sf::Vector2f& p2);
    void setThickness(float f);
    void setColor(sf::Color c);
    sf::Color getColor(void) const { return m_color; }

private:
    sf::Vertex m_vertices[4];
    float m_thickness;
    sf::Color m_color;
    sf::Vector2f m_p1;
    sf::Vector2f m_p2;

    void build(void);
};

#endif /* THICK_LINE_HPP */
