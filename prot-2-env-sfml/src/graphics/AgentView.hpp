/***********************************************************************************************//**
 *  Graphical representation of an agent.
 *  @class      AgentView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef AGENT_VIEW_HPP
#define AGENT_VIEW_HPP

#include "prot.hpp"

class AgentView : public sf::Drawable, public sf::Transformable
{
public:
    AgentView(std::string id, float swath = 0.f, float comms_range = 0.f);

    void setSwath(float s);
    void setCommsRange(float r);
    void displaySwath(bool d);
    void displayRange(bool d);
    void displayId(bool d);
    void setDirection(float r);
    void setAlpha(float a);
    void setText(std::string str);

    float getSwath(void) const { return m_swath; }
    float getCommsRange(void) const { return m_comms_range; }
    bool isSwathHidden(void) const { return m_display_swath; }
    bool isRangeHidden(void) const { return m_display_range; }
    bool isIdHidden(void) const { return m_display_id; }

private:
    float m_swath;
    float m_comms_range;
    bool m_display_swath;
    bool m_display_range;
    bool m_display_id;
    sf::ConvexShape m_triangle;
    sf::CircleShape m_footprint;
    sf::CircleShape m_range;
    sf::Text m_txt;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif /* AGENT_VIEW_HPP */
