/***********************************************************************************************//**
 *  Graphical representation of an observation segment.
 *  @class      SegmentView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "SegmentView.hpp"

SegmentView::SegmentView(const Intent& in, float swath)
    : SegmentView(in.pstart, in.pend, swath, in.getAgentId() + ":" + std::to_string(in.id))
{ }

SegmentView::SegmentView(sf::Vector2f p1, sf::Vector2f p2, float swath, std::string str)
    : m_p1(p1)
    , m_p2(p2)
    , m_active(false)
    , m_owned(false)
{
    /* Find a unitary vector perpendicular to p12: */
    sf::Vector2f p12 = p2 - p1;
    p12 = p12 / std::sqrt(p12.x * p12.x + p12.y * p12.y);
    sf::Vector2f u = { p2.y - p1.y, p1.x - p2.x };
    u /= std::sqrt(u.x * u.x + u.y * u.y);
    sf::Vector2f s1 = u * (swath / 2.f);
    sf::Vector2f s2 = u * (-swath / 2.f);

    sf::Color c = sf::Color::White;
    c.a = 180;

    /* Line 1: trajectory of the segment. */
    m_line0.setPoints(p1, p2);
    m_line0.setColor(c);
    m_line0.setThickness(2.f);

    /* Line 2: start of the segment. */
    m_line1.setPoints(s1 + p1, s2 + p1);
    m_line1.setColor(c);
    m_line1.setThickness(2.f);

    /* Line 2: end of the segment. */
    m_line2.setPoints(s1 + p2, s2 + p2);
    m_line2.setColor(c);
    m_line2.setThickness(2.f);

    m_txt.setFont(Config::fnt_monospace);
    m_txt.setString(str);
    m_txt.setCharacterSize(Config::fnt_size);
    m_txt.setColor(sf::Color::White);

    sf::FloatRect txt_rect = m_txt.getLocalBounds();
    m_txt.setOrigin(txt_rect.left + txt_rect.width / 2.0f, txt_rect.top  + txt_rect.height / 2.0f);
    m_txt.setPosition(p1 - (p12 * 30.f));
}

void SegmentView::setOwnership(bool mine)
{
    m_owned = mine;
    sf::Color c;
    if(m_owned && m_active) {
        m_line0.setColor(Config::color_orange);
        m_line1.setColor(Config::color_orange);
        m_line2.setColor(Config::color_orange);
    } else if(m_owned && !m_active) {
        c = Config::color_orange;
        c.a = 180;
        m_line0.setColor(c);
        m_line1.setColor(c);
        m_line2.setColor(c);
    } else if(!m_owned && m_active) {
        m_line0.setColor(sf::Color::White);
        m_line1.setColor(sf::Color::White);
        m_line2.setColor(sf::Color::White);
    } else if(!m_owned && !m_active) {
        c = sf::Color::White;
        c.a = 180;
        m_line0.setColor(c);
        m_line1.setColor(c);
        m_line2.setColor(c);
    }
}

void SegmentView::setActive(bool active)
{
    m_active = active;
    sf::Color c;
    if(m_active) {
        c = m_line0.getColor();
        c.a = 255;
        m_line0.setColor(c);
        m_line0.setThickness(3.f);
        c = m_line1.getColor();
        c.a = 255;
        m_line1.setColor(c);
        m_line1.setThickness(3.f);
        c = m_line2.getColor();
        c.a = 255;
        m_line2.setColor(c);
        m_line2.setThickness(3.f);
    } else {
        c = m_line0.getColor();
        c.a = 180;
        m_line0.setColor(c);
        c = m_line1.getColor();
        c.a = 180;
        m_line1.setColor(c);
        c = m_line2.getColor();
        c.a = 180;
        m_line2.setColor(c);
    }
}

void SegmentView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_line0, states);
    target.draw(m_line1, states);
    target.draw(m_line2, states);
    target.draw(m_txt, states);
}
