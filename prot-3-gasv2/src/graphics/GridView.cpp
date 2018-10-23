/***********************************************************************************************//**
 *  Graphical representation of a spatial surface.
 *  @class      GridView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-19
 *  @version    0.2
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "GridView.hpp"

GridView::GridView(int w, int h, float cw, float ch, sf::Color init_color)
    : m_width(w)
    , m_height(h)
    , m_grid(sf::Triangles, w * h * 6)
    , m_color_gradient(Config::color_gradient_krbg)
{
    int v = 0;
    m_grid_idxs.reserve(m_width);
    for(int x = 0; x < m_width; x++) {
        std::vector<GridUnit> col;
        col.reserve(m_height);
        for(int y = 0; y < m_height; y++) {
            GridUnit cell_idx;
            /* Triangle 1: */
            m_grid[v + 0] = sf::Vertex(sf::Vector2f(cw * (x + 0), ch * (y + 0)), init_color);
            m_grid[v + 1] = sf::Vertex(sf::Vector2f(cw * (x + 1), ch * (y + 0)), init_color);
            m_grid[v + 2] = sf::Vertex(sf::Vector2f(cw * (x + 0), ch * (y + 1)), init_color);
            cell_idx.ca0 = v + 0;
            cell_idx.ca1 = v + 1;
            cell_idx.ca3 = v + 2;
            /* Triangle 2: */
            m_grid[v + 3] = sf::Vertex(sf::Vector2f(cw * (x + 1), ch * (y + 0)), init_color);
            m_grid[v + 4] = sf::Vertex(sf::Vector2f(cw * (x + 1), ch * (y + 1)), init_color);
            m_grid[v + 5] = sf::Vertex(sf::Vector2f(cw * (x + 0), ch * (y + 1)), init_color);
            cell_idx.cb1 = v + 3;
            cell_idx.cb2 = v + 4;
            cell_idx.cb3 = v + 5;

            col.push_back(cell_idx);
            v += 6;
        }
        m_grid_idxs.push_back(col);
    }
}

void GridView::setColor(int x, int y, sf::Color c)
{
    m_grid[m_grid_idxs[x][y].ca0 + 0].color = c;
    m_grid[m_grid_idxs[x][y].ca0 + 1].color = c;
    m_grid[m_grid_idxs[x][y].ca0 + 2].color = c;
    m_grid[m_grid_idxs[x][y].ca0 + 3].color = c;
    m_grid[m_grid_idxs[x][y].ca0 + 4].color = c;
    m_grid[m_grid_idxs[x][y].ca0 + 5].color = c;
}

void GridView::setColor(std::vector<sf::Vector2i> units, sf::Color c)
{
    for(auto& u : units) {
        setColor(u.x, u.y, c);
    }
}

void GridView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if(m_show) {
        target.draw(m_grid, states);
    }
}

void GridView::setValue(int x, int y, float v)
{
    setColor(x, y, m_color_gradient.getColorAt(v));
}

void GridView::setValue(float v)
{
    for(int xx = 0; xx < m_width; xx++) {
        for(int yy = 0; yy < m_height; yy++) {
            setColor(xx, yy, m_color_gradient.getColorAt(v));
        }
    }
}
