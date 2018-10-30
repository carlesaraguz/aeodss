/***********************************************************************************************//**
 *  Graphical representation of an observation segment.
 *  @class      SegmentView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "SegmentView.hpp"

CREATE_LOGGER(SegmentView)

SegmentView::SegmentView(const Activity& /* a */)
{
    Log::err << "Error, unimplemented constructor.\n";
    std::exit(-1);
}

SegmentView::SegmentView(std::vector<sf::Vector2f> ps, std::string str)
    : m_positions(ps)
    , m_active(false)
    , m_owned(true)
    , m_done(false)
    , m_error(false)
{
    if(m_positions.size() < 2) {
        Log::err << "Segment view error: can't define a segment with less than two positions.\n";
        m_error = true;
    }
    auto p_s0 = m_positions[0];
    auto p_s1 = m_positions[1];
    // auto p_e0 = m_positions[m_positions.size() - 2];
    // auto p_e1 = m_positions[m_positions.size() - 1];

    // /* Find a unitary vectors perpendicular to the start and end directions: */
    sf::Vector2f p0 = p_s1 - p_s0;
    // sf::Vector2f p1 = p_e1 - p_e0;
    p0 = p0 / std::sqrt(p0.x * p0.x + p0.y * p0.y);
    // p1 = p1 / std::sqrt(p1.x * p1.x + p1.y * p1.y);
    //
    // sf::Vector2f u0 = { p_s1.y - p_s0.y, p_s0.x - p_s1.x };
    // sf::Vector2f u1 = { p_e1.y - p_e0.y, p_e0.x - p_e1.x };
    // u0 /= std::sqrt(u0.x * u0.x + u0.y * u0.y);
    // u1 /= std::sqrt(u1.x * u1.x + u1.y * u1.y);
    // sf::Vector2f p_sa = u0 * (swath / 2.f);
    // sf::Vector2f p_sb = u0 * (-swath / 2.f);
    // sf::Vector2f p_ea = u1 * (swath / 2.f);
    // sf::Vector2f p_eb = u1 * (-swath / 2.f);

    sf::Color c = sf::Color::White;
    c.a = 180;

    /* Lines: trajectory of the segment. */
    Log::err << "**** pos size: " << m_positions.size() << "\n";
    Log::err << "**** x: " << m_positions.front().x <<  "\n";

    for(unsigned int i = 1; i < m_positions.size(); i++) {
        if((m_positions[i - 1].x <= 0.f && m_positions[i - 1].y <= 0.f) || (m_positions[i].x <= 0.f && m_positions[i].y <= 0.f)) {
            Log::err << "Segment view error: found a potential inconsistency in position {" << i << "}\n";
            Log::err << "***** x: " << m_positions[i - 1].x << " y: " << m_positions[i - 1].y << "\n";
            m_error = true;
        }
        ThickLine tline(m_positions[i - 1], m_positions[i]);
        tline.setColor(c);
        tline.setThickness(2.f);
        m_lines.push_back(tline);
    }

    float circle_radius = 7.f;
    /* Circle: start of the segment. */
    m_circle_start.setRadius(circle_radius);
    m_circle_start.setFillColor(c);
    m_circle_start.setPosition(m_positions[0] - sf::Vector2f(circle_radius, circle_radius));

    /* Circle: end of the segment. */
    m_circle_end.setRadius(circle_radius);
    m_circle_end.setFillColor(c);
    m_circle_end.setPosition(m_positions[m_positions.size() - 1] - sf::Vector2f(circle_radius, circle_radius));

    m_txt.setFont(Config::fnt_monospace);
    m_txt.setString(str);
    m_txt.setCharacterSize(Config::fnt_size);
    m_txt.setFillColor(sf::Color::White);

    sf::FloatRect txt_rect = m_txt.getLocalBounds();
    m_txt.setOrigin(txt_rect.left + txt_rect.width / 2.0f, txt_rect.top  + txt_rect.height / 2.0f);
    m_txt.setPosition(p_s0 - (p0 * 30.f));
}

void SegmentView::decorate(void)
{
    sf::Color c;

    /* Color: */
    if(m_owned) {
        c = sf::Color::White;
    } else {
        c = Config::color_orange;
    }

    /* Alpha: */
    if(m_active) {
        c.a = 255 * 1.f;
    } else if(m_done) {
        c.a = 255 * 0.25f;
    } else {
        c.a = 255 * 0.5f;
    }

    for(auto& l : m_lines) {
        l.setColor(c);
    }
    m_circle_start.setFillColor(c);
    m_circle_end.setFillColor(c);
    m_txt.setFillColor(c);
}

void SegmentView::setOwnership(bool mine)
{
    m_owned = mine;
    decorate();
}

void SegmentView::setActive(bool active)
{
    m_active = active;
    m_done = false;
    if(m_active) {
        for(auto& l : m_lines) {
            l.setThickness(3.f);
        }
        m_circle_start.setRadius(15.f);
        m_circle_end.setRadius(15.f);
    } else {
        for(auto& l : m_lines) {
            l.setThickness(2.f);
        }
        m_circle_start.setRadius(10.f);
        m_circle_end.setRadius(10.f);
    }
    decorate();
}

void SegmentView::setDone(bool done)
{
    if(!m_done && done) {
        setActive(false);
        m_done = true;
        decorate();
    }
}

void SegmentView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if(m_show) {
        target.draw(m_circle_start, states);
        target.draw(m_circle_end, states);
        for(auto& l : m_lines) {
            target.draw(l, states);
        }
        target.draw(m_txt, states);
    }
}