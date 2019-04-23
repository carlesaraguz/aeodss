/***********************************************************************************************//**
 *  Graphical representation of an agent.
 *  @class      AgentView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "AgentView.hpp"

AgentView::AgentView(std::string str, float comms_range)
    : m_comms_range(comms_range)
    , m_range(comms_range, 20)
    , m_triangle(3)
    , m_display_id(true)
    , m_display_range(true)
    , m_display_footprint(true)
{
    m_triangle.setPoint(0, sf::Vector2f(Config::agent_size, 0.f));
    m_triangle.setPoint(2, sf::Vector2f(
        Config::agent_size * std::cos(-2.f * Config::pi / 3.0),
        Config::agent_size * std::sin(-2.f * Config::pi / 3.0) * 0.5f
    ));
    m_triangle.setPoint(1, sf::Vector2f(
        Config::agent_size * std::cos(2.0 * Config::pi / 3.0),
        Config::agent_size * std::sin(2.0 * Config::pi / 3.0) * 0.5f
    ));

    m_range.setOrigin(m_comms_range, m_comms_range);
    m_txt.setPosition(Config::agent_size, Config::agent_size);

    m_range.setFillColor(sf::Color::Transparent);
    m_triangle.setFillColor(Config::color_orange);

    m_range.setOutlineColor(Config::color_orange);
    m_range.setOutlineThickness(2.f);
    m_triangle.setOutlineColor(sf::Color::Transparent);

    m_txt.setFont(Config::fnt_monospace);
    m_txt.setString(str);
    m_txt.setCharacterSize(Config::fnt_size);
    m_txt.setFillColor(sf::Color::White);
}

void AgentView::setFootprint(std::vector<sf::Vector2f> footprint)
{
    m_footprint.clear();
    for(int i = 1; i < (int)footprint.size(); i++) {
        ThickLine tl(footprint[i - 1], footprint[i]);
        tl.setThickness(2.f);
        // tl.setColor(Config::color_dark_green);
        tl.setColor(Config::color_orange);
        // tl.setColor(sf::Color::White);
        m_footprint.push_back(tl);
    }
}

void AgentView::setCommsRange(float r)
{
    m_comms_range = r;
    m_range.setRadius(m_comms_range);
    m_range.setOrigin(m_comms_range, m_comms_range);
}

void AgentView::displayFootprint(bool d)
{
    m_display_footprint = d;
}

void AgentView::displayRange(bool d)
{
    m_display_range = d;
}

void AgentView::displayId(bool d)
{
    m_display_id = d;
}

void AgentView::setDirection(sf::Vector2f vel)
{
    float dir = 0.f;
    switch(quadrant(vel)) {
        case 1:
        case 2:
        dir = std::acos(vel.x);
        break;
        case 3:
        case 4:
        dir = -std::acos(vel.x);
        break;
    }
    dir *= 180.f / Config::pi;
    m_triangle.setRotation(dir);
}

unsigned int AgentView::quadrant(sf::Vector2f v) const
{
    if(v.x >= 0 && v.y >= 0) {
        return 1;
    } else if(v.x <= 0 && v.y >= 0) {
        return 2;
    } else if(v.x <= 0 && v.y <= 0) {
        return 3;
    } else if(v.x >= 0 && v.y <= 0) {
        return 4;
    } else {
        return 0;
    }
}

void AgentView::setAlpha(float a)
{
    sf::Color c1 = Config::color_orange;
    sf::Color c2 = Config::color_dark_green;
    c1.a = c2.a = a * 255;
    m_triangle.setFillColor(c1);
    m_range.setOutlineColor(c1);
    for(auto& tl : m_footprint) {
        tl.setColor(c2);
    }
}

void AgentView::setText(std::string str)
{
    m_txt.setString(str);
}

void AgentView::setLocation(sf::Vector2f l)
{
    m_txt.setPosition(l.x + Config::agent_size, l.y + Config::agent_size);
    m_range.setPosition(l);
    m_triangle.setPosition(l);
}

void AgentView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if(m_show) {
        if(m_display_footprint) {
            for(auto& f_line : m_footprint) {
                target.draw(f_line, states);
            }
        }
        if(m_display_id) {
            target.draw(m_txt, states);
        }
        if(m_display_range) {
            target.draw(m_range, states);
        }
        target.draw(m_triangle, states);
    }
}
