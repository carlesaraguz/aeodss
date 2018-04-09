/***********************************************************************************************//**
 *  Graphical representation of the environment model or state.
 *  @class      EnvModelView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef ENV_MODEL_VIEW_HPP
#define ENV_MODEL_VIEW_HPP

#include "prot.hpp"
#include "ColorGradient.hpp"

class EnvModel;

class EnvModelView : public sf::Drawable
{
public:
    EnvModelView(int modelw, int modelh, int worldw, int worldh, sf::Color init_color = sf::Color::Black);
    EnvModelView(const EnvModel& e, sf::Color init_color = sf::Color::Black);

    void display(const EnvModel& e, unsigned int layer = 0);
    void setColorGradient(const ColorGradient& cg) { m_color_gradient = cg; }

private:
    struct EMCellView {
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

    int m_model_w;
    int m_model_h;
    int m_world_w;
    int m_world_h;
    sf::VertexArray m_grid;
    std::vector<std::vector<EMCellView> > m_grid_idxs;
    ColorGradient m_color_gradient;

    void setColor(int x, int y, sf::Color c);
    void setColor(std::vector<std::tuple<int,int> > units, sf::Color c);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#include "EnvModel.hpp"

#endif /* ENV_MODEL_VIEW_HPP */
