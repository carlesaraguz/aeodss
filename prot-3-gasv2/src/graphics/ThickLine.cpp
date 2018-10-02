/***********************************************************************************************//**
 *  A line with thickness.
 *  @class      ThickLine
 *  @authors    Tina @ https://github.com/SFML/SFML/wiki/Source:-Line-segment-with-thickness
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "ThickLine.hpp"

ThickLine::ThickLine(const sf::Vector2f& p1, const sf::Vector2f& p2)
    : m_color(sf::Color::Transparent)
    , m_thickness(1.f)
    , m_p1(p1)
    , m_p2(p2)
{
    build();
}

void ThickLine::build(void)
{
    sf::Vector2f dir = m_p2 - m_p1;
    sf::Vector2f udir = dir / std::sqrt(dir.x * dir.x + dir.y * dir.y);
    sf::Vector2f v(-udir.y, udir.x);

    sf::Vector2f offset = (m_thickness / 2.f) * v;

    m_vertices[0].position = m_p1 + offset;
    m_vertices[1].position = m_p2 + offset;
    m_vertices[2].position = m_p2 - offset;
    m_vertices[3].position = m_p1 - offset;

    for(int i = 0; i < 4; i++) {
        m_vertices[i].color = m_color;
    }
}

void ThickLine::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    target.draw(m_vertices, 4, sf::Quads, states);
}

void ThickLine::setPoints(const sf::Vector2f& p1, const sf::Vector2f& p2)
{
    m_p1 = p1;
    m_p2 = p2;
    build();
}

void ThickLine::setThickness(float f)
{
    m_thickness = f;
    build();
}

void ThickLine::setColor(sf::Color c)
{
    m_color = c;
    for(int i = 0; i < 4; i++) {
        m_vertices[i].color = m_color;
    }
}
