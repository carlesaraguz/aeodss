/***********************************************************************************************//**
 *  Graphical representation of a spatial surface.
 *  @class      GridView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-19
 *  @version    0.2
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef GRID_VIEW_HPP
#define GRID_VIEW_HPP

#include "prot.hpp"
#include "ColorGradient.hpp"
#include "HideGraphics.hpp"

class GridView : public HideGraphics, public sf::Drawable
{
public:
    GridView(int w, int h, float cw, float ch, sf::Color init_color = sf::Color::Black);

    void setColorGradient(const ColorGradient& cg) { m_color_gradient = cg; }
    void setValue(int x, int y, float v);
    void setValue(float v);

private:
    struct GridUnit {
        /*  Indices to the corners (i.e. vertices in m_grid) of a cell.
         *  Corners are numbered clockwise starting at top-left.
         *  Letter A or B (caX and cbX) correspond to the triangle (top-left:A, bottom-right:B)
         **/
        int ca0;
        int ca1;
        int cb1;
        int cb2;
        int ca3;
        int cb3;
    };

    int m_width;
    int m_height;
    sf::VertexArray m_grid;
    std::vector<std::vector<GridUnit> > m_grid_idxs;
    ColorGradient m_color_gradient;

    void setColor(int x, int y, sf::Color c);
    void setColor(std::vector<sf::Vector2i> units, sf::Color c);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif /* GRID_VIEW_HPP */
