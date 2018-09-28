/***********************************************************************************************//**
 *  Graphical representation of an agent.
 *  @class      AgentView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef AGENT_VIEW_HPP
#define AGENT_VIEW_HPP

#include "prot.hpp"
#include "HideGraphics.hpp"
#include "ThickLine.hpp"

class AgentView : public HideGraphics, public sf::Drawable, public sf::Transformable
{
public:
    AgentView(std::string id, float comms_range = 0.f);

    void setFootprint(std::vector<sf::Vector2f> footprint);
    void setCommsRange(float r);
    void displayFootprint(bool d = true);
    void displayRange(bool d = true);
    void displayId(bool d = true);
    void setDirection(sf::Vector2f vel);
    void setAlpha(float a);
    void setText(std::string str);

    float getCommsRange(void) const { return m_comms_range; }
    bool isFootprintHidden(void) const { return m_display_footprint; }
    bool isRangeHidden(void) const { return m_display_range; }
    bool isIdHidden(void) const { return m_display_id; }

private:
    float m_comms_range;
    bool m_display_footprint;
    bool m_display_range;
    bool m_display_id;
    sf::ConvexShape m_triangle;
    std::vector<ThickLine> m_footprint;
    sf::CircleShape m_range;
    sf::Text m_txt;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    unsigned int quadrant(sf::Vector2f v) const;
};

#endif /* AGENT_VIEW_HPP */
