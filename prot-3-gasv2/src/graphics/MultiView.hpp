/***********************************************************************************************//**
 *  A wrapper for multiple graphically representable objects.
 *  @class      MultiView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef MULTI_VIEW_HPP
#define MULTI_VIEW_HPP

#include "prot.hpp"
#include "HasView.hpp"

class MultiView : public sf::Drawable, public sf::Transformable
{
public:
    MultiView(std::vector<std::shared_ptr<const HasView> > views = { });
    void setViews(std::vector<std::shared_ptr<const HasView> > views) { m_views = views; }
    void addViewToFront(std::shared_ptr<const HasView> view);
    void addViewToBack(std::shared_ptr<const HasView> view);
    void drawViews(void);

private:
    sf::RenderTexture m_target_texture;
    sf::RectangleShape m_border;
    std::vector<std::shared_ptr<const HasView> > m_views;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif /* MULTI_VIEW_HPP */
