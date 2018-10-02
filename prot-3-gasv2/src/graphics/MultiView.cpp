/***********************************************************************************************//**
 *  A wrapper for multiple graphically representable objects.
 *  @class      MultiView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "MultiView.hpp"

MultiView::MultiView(std::vector<std::shared_ptr<const HasView> > views)
    : m_views(views)
{
    m_border.setSize(sf::Vector2f(Config::world_width, Config::world_height));
    m_border.setFillColor(sf::Color::Transparent);
    m_border.setOutlineColor(sf::Color::White);
    m_border.setOutlineThickness(1.f);

    m_target_texture.create(Config::world_width, Config::world_height);
    m_target_texture.setSmooth(true);
}

void MultiView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    /* Draw the elements painted with the texture: */
    states.transform *= getTransform();
    sf::Sprite sprite(m_target_texture.getTexture());
    target.draw(sprite, states);
    target.draw(m_border, states);
}

void MultiView::addViewToBack(std::shared_ptr<const HasView> view)
{
    m_views.push_back(view);
}

void MultiView::addViewToFront(std::shared_ptr<const HasView> view)
{
    m_views.insert(m_views.begin(), view);
}

void MultiView::drawViews(void)
{
    m_target_texture.clear();
    for(const auto& v : m_views) {
        m_target_texture.draw(v->getView());
    }
    m_target_texture.display();
}
