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

EnvModelView::EnvModelView(int w, int h, sf::Color init_color)
    : m_width(w)
    , m_height(h)
{
    sf::Image image;
    image.create(m_width, m_height, init_color);
    m_texture.loadFromImage(image);
    m_texture.setSmooth(false);
    m_sprite.setTexture(m_texture);
    m_sprite.setScale(
        (float)Config::world_width / (float)m_width,
        (float)Config::world_height / (float)m_height
    );
}

EnvModelView::EnvModelView(const EnvModel& e)
    : m_width(e.getModelWidth())
    , m_height(e.getModelHeight())
    , m_grid(sf::LinesStrip, 3)
{
    sf::Image image;
    image.create(m_width, m_height, sf::Color::Black);
    sf::Color c(0, 0, 0, 255);
    for(int y = 0; y < m_height; y++) {
        for(int x = 0; x < m_width; x++) {
            c.r = e.getValue(x, y);
            c.g = e.getValue(x, y);
            c.b = e.getValue(x, y);
            image.setPixel(x, y, c);
        }
    }
    m_texture.loadFromImage(image);
    m_texture.setSmooth(false);
    m_sprite.setTexture(m_texture);
    m_sprite.setScale(
        (float)Config::world_width / (float)m_width,
        (float)Config::world_height / (float)m_height
    );

    m_grid[0] = sf::Vertex(sf::Vector2f(0.f, 0.f), sf::Color::White);
    m_grid[1] = sf::Vertex(sf::Vector2f(Config::world_width, 0.f), sf::Color::White);
    m_grid[2] = sf::Vertex(sf::Vector2f(0.f, Config::world_height), sf::Color::White);
}

EnvModelView::EnvModelView(const EnvModel& e, sf::Color init_color)
    : EnvModelView(e.getModelWidth(), e.getModelHeight(), init_color)
{ }

void EnvModelView::setColor(int x, int y, sf::Color c)
{
    sf::Uint8* pixels = new sf::Uint8[4];
    pixels[0] = c.r;
    pixels[1] = c.g;
    pixels[2] = c.b;
    pixels[3] = c.a;
    m_texture.update(pixels, 1, 1, x, y);
    m_sprite.setTexture(m_texture);
    delete[] pixels;
}

void EnvModelView::setColor(std::vector<std::tuple<int,int> > units, sf::Color c)
{
    for(auto& u : units) {
        setColor(std::get<0>(u), std::get<1>(u), c);
    }
}

void EnvModelView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    // target.draw(m_sprite, states);
    target.draw(m_grid, states);
}

void EnvModelView::display(const EnvModel& e, unsigned int layer)
{
    sf::Image image;
    image.create(m_width, m_height, sf::Color::Black);
    sf::Color c(0, 0, 0, 255);
    for(int y = 0; y < m_height; y++) {
        for(int x = 0; x < m_width; x++) {
            c.r = e.getValue(x, y, layer);
            c.g = e.getValue(x, y, layer);
            c.b = e.getValue(x, y, layer);
            image.setPixel(x, y, c);
        }
    }
    m_texture.update(image);
}
