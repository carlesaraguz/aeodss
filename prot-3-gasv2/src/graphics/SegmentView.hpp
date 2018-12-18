/***********************************************************************************************//**
 *  Graphical representation of an observation segment.
 *  @class      SegmentView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef SEGMENT_VIEW_HPP
#define SEGMENT_VIEW_HPP

#include "prot.hpp"

#include "MathUtils.hpp"
#include "ThickLine.hpp"
#include "Activity.hpp"
#include "HideGraphics.hpp"

class SegmentView : public HideGraphics, public sf::Drawable
{
public:
    SegmentView(const Activity& a);
    SegmentView(std::vector<sf::Vector2f> ps, std::string str = "");

    void setOwnership(bool mine);
    void setActive(bool active);
    void setDone(bool done);

private:
    float m_swath;
    std::vector<sf::Vector2f> m_positions;
    bool m_done;
    bool m_active;
    bool m_owned;
    std::vector<ThickLine> m_lines;
    sf::CircleShape m_circle_start;
    sf::CircleShape m_circle_end;
    sf::Text m_txt;

    bool m_error;

    void decorate(void);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif /* SEGMENT_VIEW_HPP */
