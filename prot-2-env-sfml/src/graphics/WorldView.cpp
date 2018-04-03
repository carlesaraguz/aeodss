/***********************************************************************************************//**
 *  A wrapper for a rendering target of a number of agents.
 *  @class      WorldView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "WorldView.hpp"

WorldView::WorldView(WorldViewType type, std::vector<std::shared_ptr<Agent> > agents)
    : m_type(type)
    , m_agents(agents)
{
    m_border.setSize(sf::Vector2f(Config::world_width, Config::world_height));
    m_border.setFillColor(sf::Color::Transparent);
    m_border.setOutlineColor(sf::Color::White);
    m_border.setOutlineThickness(1.f);

    m_target_texture.create(Config::world_width, Config::world_height);
    m_target_texture.setSmooth(true);
    m_canvas.setTexture(m_target_texture.getTexture());
}

void WorldView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    /* Draw the elements painted with the texture: */
    states.transform *= getTransform();
    sf::Sprite sprite(m_target_texture.getTexture());
    target.draw(sprite, states);
    // target.draw(m_canvas, states);
    target.draw(m_border, states);
}

void WorldView::drawWorld(void)
{
    m_target_texture.clear();
    switch(m_type) {
        case WorldViewType::SINGLE_AGENT:
            if(m_agents.size() >= 1) {
                m_target_texture.draw(m_agents[0]->getEnvView());
                m_target_texture.draw(m_agents[0]->getAgentView());
                auto segments = m_agents[0]->getSegmentViews();
                for(const auto& seg : segments) {
                    m_target_texture.draw(seg.second);
                }
            }
            break;
        case WorldViewType::GLOBAL_REAL:
        case WorldViewType::GLOBAL_ORACLE:
        default:
            for(const auto& a : m_agents) {
                m_target_texture.draw(a->getAgentView());
                auto segments = a->getSegmentViews();
                for(const auto& seg : segments) {
                    m_target_texture.draw(seg.second);
                }
            }
            break;
    }
    m_target_texture.display();
}
