/***********************************************************************************************//**
 *  Graphical representation of the environment model or state.
 *  @class      EnvModelView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "EnvModelView.hpp"

EnvModelView::EnvModelView(int modelw, int modelh, int worldw, int worldh, sf::Color init_color)
    : m_model_w(modelw)
    , m_model_h(modelh)
    , m_world_w(worldw)
    , m_world_h(worldh)
    , m_grid(sf::Triangles, modelw * modelh * 6)
    , m_color_gradient(Config::color_gradient_blue)
{
    float cell_w = (float)m_world_w / (float)m_model_w;
    float cell_h = (float)m_world_h / (float)m_model_h;
    int v = 0;
    m_grid_idxs.reserve(m_model_w);
    for(int x = 0; x < m_model_w; x++) {
        std::vector<EMCellView> col;
        col.reserve(m_model_h);
        for(int y = 0; y < m_model_h; y++) {
            EMCellView cell_idx;
            /* Triangle 1: */
            m_grid[v + 0] = sf::Vertex(sf::Vector2f(cell_w * (x + 0), cell_h * (y + 0)), init_color);
            m_grid[v + 1] = sf::Vertex(sf::Vector2f(cell_w * (x + 1), cell_h * (y + 0)), init_color);
            m_grid[v + 2] = sf::Vertex(sf::Vector2f(cell_w * (x + 0), cell_h * (y + 1)), init_color);
            cell_idx.ca0 = v + 0;
            cell_idx.ca1 = v + 1;
            cell_idx.ca3 = v + 2;
            /* Triangle 2: */
            m_grid[v + 3] = sf::Vertex(sf::Vector2f(cell_w * (x + 1), cell_h * (y + 0)), init_color);
            m_grid[v + 4] = sf::Vertex(sf::Vector2f(cell_w * (x + 1), cell_h * (y + 1)), init_color);
            m_grid[v + 5] = sf::Vertex(sf::Vector2f(cell_w * (x + 0), cell_h * (y + 1)), init_color);
            cell_idx.cb1 = v + 3;
            cell_idx.cb2 = v + 4;
            cell_idx.cb3 = v + 5;

            col.push_back(cell_idx);
            v += 6;
        }
        m_grid_idxs.push_back(col);
    }
}

EnvModelView::EnvModelView(const EnvModel& e, sf::Color init_color)
    : EnvModelView(e.getModelWidth(), e.getModelHeight(), e.getWorldWidth(), e.getWorldHeight(), init_color)
{ }

void EnvModelView::setColor(int x, int y, sf::Color c)
{
    m_grid[m_grid_idxs[x][y].ca0 + 0].color = c;
    m_grid[m_grid_idxs[x][y].ca0 + 1].color = c;
    m_grid[m_grid_idxs[x][y].ca0 + 2].color = c;
    m_grid[m_grid_idxs[x][y].ca0 + 3].color = c;
    m_grid[m_grid_idxs[x][y].ca0 + 4].color = c;
    m_grid[m_grid_idxs[x][y].ca0 + 5].color = c;
}

void EnvModelView::setColor(std::vector<std::tuple<int,int> > units, sf::Color c)
{
    for(auto& u : units) {
        setColor(std::get<0>(u), std::get<1>(u), c);
    }
}

void EnvModelView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if(m_show) {
        target.draw(m_grid, states);
    }
}

void EnvModelView::display(const EnvModel& e, unsigned int layer)
{
    if(m_show) {
        float val;
        for(int x = 0; x < m_model_w; x++) {
            for(int y = 0; y < m_model_h; y++) {
                val = e.getValueByModelCoord(x, y, layer);
                setColor(x, y, m_color_gradient.getColorAt(val));
            }
        }
    }
}
