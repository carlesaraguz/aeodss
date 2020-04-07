/***********************************************************************************************//**
 *  Graphical representation of a link and information transfer.
 *  @class      AgentLinkView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-dec-18
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef AGENT_LINK_VIEW_HPP
#define AGENT_LINK_VIEW_HPP

#include "prot.hpp"
#include "HideGraphics.hpp"
#include "ThickLine.hpp"

class AgentLinkView : public HideGraphics, public sf::Drawable
{
public:
    enum class State {
        LINE_OF_SIGHT,  /**< There has not been a link established but agents are in view. */
        DISCONNECTED,   /**< The link has been destroyed and agents are not in view. */
        CONNECTED,      /**< Connected, but not sending (perhaps receiving). */
        SENDING         /**< Connected and trasfering packets. */
    };

    /*******************************************************************************************//**
     *  Create a new link view.
     **********************************************************************************************/
    AgentLinkView(void) = default;

    /*******************************************************************************************//**
     *  Define the current position for this link/agent (i.e. where lines will be connected to).
     *  @param  l   Location (in 2d world pixels/projection) of the agent.
     **********************************************************************************************/
    void setPosition(sf::Vector2f l) { m_position = l; }

    /*******************************************************************************************//**
     *  Define the current position for this link/agent (i.e. where lines will be connected to).
     *  @param  l   Location (in 3d) of the agent.
     **********************************************************************************************/
    void setPosition(sf::Vector3f l);

    /*******************************************************************************************//**
     *  Add a new link or modify its status or position.
     *  @param  aid     ID of the agent to display link with.
     *  @param  s       The status of this link. Different status will be displayed differently.
     *  @param  pos     Location of the other agent (in 2d world pixels/projection).
     **********************************************************************************************/
    void setLink(std::string aid, State s, sf::Vector2f pos = {});

    /*******************************************************************************************//**
     *  Add a new link or modify its status or position.
     *  @param  aid     ID of the agent to display link with.
     *  @param  s       The status of this link. Different status will be displayed differently.
     *  @param  pos     Location of the other agent (in 3d).
     **********************************************************************************************/
    void setLink(std::string aid, State s, sf::Vector3f pos);

    /*******************************************************************************************//**
     *  Erases all visual elements, i.e. ThickLines. It doesn't remove control structures.
     **********************************************************************************************/
    void removeAll(void);

private:
    sf::Vector2f m_position;
    std::map<std::string, sf::Vector2f> m_link_targets;
    std::map<std::string, State> m_link_states;
    std::map<std::string, ThickLine> m_link_lines;

    void updateLine(std::string aid);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif /* AGENT_VIEW_HPP */
