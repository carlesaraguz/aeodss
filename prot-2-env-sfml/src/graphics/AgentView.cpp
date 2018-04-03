/***********************************************************************************************//**
 *  Graphical representation of an agent.
 *  @class      AgentView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "AgentView.hpp"

AgentView::AgentView(std::string str, float swath, float comms_range)
    : m_swath(swath)
    , m_comms_range(comms_range)
    , m_range(comms_range, 20)
    , m_footprint(swath / 2.f, 20)
    , m_triangle(3)
    , m_display_id(true)
    , m_display_range(true)
    , m_display_swath(true)
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
    m_footprint.setOrigin(m_swath / 2.f, m_swath / 2.f);
    m_txt.setPosition(Config::agent_size, Config::agent_size);

    m_range.setFillColor(sf::Color::Transparent);
    m_footprint.setFillColor(sf::Color::Transparent);
    m_triangle.setFillColor(Config::color_orange);

    m_range.setOutlineColor(Config::color_orange);
    m_range.setOutlineThickness(2.f);
    m_footprint.setOutlineColor(Config::color_dark_green);
    m_footprint.setOutlineThickness(2.f);
    m_triangle.setOutlineColor(sf::Color::Transparent);

    m_txt.setFont(Config::fnt_monospace);
    m_txt.setString(str);
    m_txt.setCharacterSize(Config::fnt_size);
    m_txt.setColor(sf::Color::White);
}

void AgentView::setSwath(float s)
{
    m_swath = s;
    m_footprint.setRadius(m_swath / 2.f);
    m_footprint.setOrigin(m_swath / 2.f, m_swath / 2.f);
}

void AgentView::setCommsRange(float r)
{
    m_comms_range = r;
    m_range.setRadius(m_comms_range);
    m_range.setOrigin(m_comms_range, m_comms_range);
}

void AgentView::displaySwath(bool d)
{
    m_display_swath = d;
}

void AgentView::displayRange(bool d)
{
    m_display_range = d;
}

void AgentView::displayId(bool d)
{
    m_display_id = d;
}

void AgentView::setDirection(float r)
{
    m_triangle.setRotation(r);
}

void AgentView::setAlpha(float a)
{
    sf::Color c1 = Config::color_orange;
    sf::Color c2 = Config::color_dark_green;
    c1.a = c2.a = a * 255;
    m_triangle.setFillColor(c1);
    m_range.setOutlineColor(c1);
    m_footprint.setOutlineColor(c2);
}

void AgentView::setText(std::string str)
{
    m_txt.setString(str);
}

void AgentView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    if(m_display_swath) {
        target.draw(m_footprint, states);
    }
    if(m_display_id) {
        target.draw(m_txt, states);
    }
    if(m_display_range) {
        target.draw(m_range, states);
    }
    target.draw(m_triangle, states);
}
