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
    , m_discarded(false)
{
    if(m_positions.size() < 2) {
        Log::err << "Segment view error: can't define a segment with less than two positions.\n";
        m_error = true;
    }
    auto p_s0 = m_positions[0];
    auto p_s1 = m_positions[1];

    sf::Vector2f p0 = MathUtils::makeUnitary(p_s1 - p_s0);
    sf::Color c = sf::Color::White;
    c.a = 180;

    /* Lines: trajectory of the segment. */
    for(unsigned int i = 1; i < m_positions.size(); i++) {
        auto p0 = m_positions[i - 1];
        auto p1 = m_positions[i];
        if( (p0.x < 0.f && p0.y < 0.f) ||
            (p1.x < 0.f && p1.y < 0.f) ||
            (p0.x > Config::world_width && p0.y > Config::world_height) ||
            (p1.x > Config::world_width && p1.y > Config::world_height)
        ) {
            Log::err << "Segment view error: found a potential inconsistency in position {" << i << "} -> "
                << "(" << m_positions[i].x << ", " << m_positions[i].y << ")\n";
            m_error = true;
        }
        auto dvec = p1 - p0;
        float dist = MathUtils::norm(dvec);
        if(dist > (3.0 / 4.0) * Config::world_width) {
            /* This line possibly crosses all screen length, skipping: */
        } else {
            ThickLine tline(p0, p1);
            tline.setColor(c);
            tline.setThickness(2.f);
            m_lines.push_back(tline);
        }
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
    if(m_owned && m_active) {
        c = sf::Color(94, 238, 255);    /* Bright blue. */
    } else if(m_owned && !m_active && !m_discarded) {
        c = sf::Color::White;
    } else if(m_owned && !m_active && m_discarded) {
        c = sf::Color::Red;
    } else if(!m_owned && !m_discarded) {
        c = sf::Color(127, 127, 127);   /* Gray. */
    } else if(!m_owned && m_discarded) {
        c = sf::Color(127, 0, 0);       /* Dark red. */
    } else {
        c = sf::Color::Green;
        Log::err << "Unexpected segment state.\n";
    }

    if(m_circle_start.getFillColor() != c) {
        /* Has to change colours: */
        for(auto& l : m_lines) {
            l.setColor(c);
        }
        m_circle_start.setFillColor(c);
        m_circle_end.setFillColor(c);
        m_txt.setFillColor(c);
    }
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
    } else {
        for(auto& l : m_lines) {
            l.setThickness(2.f);
        }
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

void SegmentView::setDiscarded(void)
{
    if(!m_discarded) {
        setActive(false);
        m_discarded = true;
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
