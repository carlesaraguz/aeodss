/***********************************************************************************************//**
 *  Graphical representation of a link and information transfer.
 *  @class      AgentLinkView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-dec-18
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "AgentLinkView.hpp"
#include "AgentMotion.hpp"

CREATE_LOGGER(AgentLinkView)

void AgentLinkView::setLink(std::string aid, State s, sf::Vector3f pos)
{
    setLink(aid, s, AgentMotion::getProjection2D(pos, VirtualTime::now()));
}

void AgentLinkView::setLink(std::string aid, State s, sf::Vector2f pos) {
    if(Config::motion_model == AgentMotionType::ORBITAL) {
        float d0 = MathUtils::norm(m_position - pos);
        float d1 = MathUtils::norm(m_position - (pos + sf::Vector2f(Config::world_width, 0.f)));
        float d2 = MathUtils::norm(m_position - (pos - sf::Vector2f(Config::world_width, 0.f)));
        if(d0 > d1 && d2 > d0) {
            pos.x += Config::world_width;
        } else if(d0 > d2 && d1 > d0) {
            pos.x -= Config::world_width;
        }
    }
    m_link_targets[aid] = pos;
    m_link_states[aid] = s;
    updateLine(aid);
}

void AgentLinkView::setPosition(sf::Vector3f l)
{
    setPosition(AgentMotion::getProjection2D(l, VirtualTime::now()));
}

void AgentLinkView::updateLine(std::string aid)
{
    if(m_link_states.at(aid) != State::DISCONNECTED) {
        /* Compute displacement: */
        sf::Vector2f dir = m_position - m_link_targets[aid];
        sf::Vector2f udir = MathUtils::makeUnitary(dir);
        sf::Vector2f v(-udir.y, udir.x);
        sf::Vector2f p0 = m_position + (v * 2.f);
        sf::Vector2f p1 = m_link_targets[aid] + (v * 2.f);

        m_link_lines[aid] = ThickLine(p0, p1);
        m_link_lines[aid].setThickness(2.f);

        switch(m_link_states[aid]) {
            case State::LINE_OF_SIGHT:
                m_link_lines[aid].setColor(Config::color_link_los);
                break;
            case State::CONNECTED:
                m_link_lines[aid].setColor(Config::color_link_connected);
                break;
            case State::SENDING:
                m_link_lines[aid].setColor(Config::color_link_sending);
                break;
            default:
                /* Does nothing. Prevents warning message in G++. */
                break;
        }
    } else {
        auto it = m_link_lines.find(aid);
        if(it != m_link_lines.end()) {
            m_link_lines.erase(it);
        }
    }
}

void AgentLinkView::removeAll(void)
{
    m_link_lines.clear();
}


void AgentLinkView::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for(auto& l : m_link_lines) {
        if(m_link_states.at(l.first) != State::DISCONNECTED) {
            target.draw(l.second, states);
        }
    }
}
