/***********************************************************************************************//**
 *  Graphical representation of an observation segment.
 *  @class      SegmentView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef SEGMENT_VIEW_HPP
#define SEGMENT_VIEW_HPP

#include "prot.hpp"

#include "ThickLine.hpp"
#include "Intent.hpp"

class SegmentView : public sf::Drawable
{
public:
    SegmentView(const Intent& in, float swath);
    SegmentView(sf::Vector2f p1, sf::Vector2f p2, float swath, std::string str = "");

    void setOwnership(bool mine);
    void setActive(bool active);

private:
    float m_swath;
    sf::Vector2f m_p1;
    sf::Vector2f m_p2;
    bool m_active;
    bool m_owned;
    ThickLine m_line0;
    ThickLine m_line1;
    ThickLine m_line2;
    sf::Text m_txt;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif /* SEGMENT_VIEW_HPP */
