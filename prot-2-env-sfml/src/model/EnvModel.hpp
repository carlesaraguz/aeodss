/***********************************************************************************************//**
 *  The modeled environment.
 *  @class      EnvModel
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-28
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef ENV_MODEL_HPP
#define ENV_MODEL_HPP

#include "prot.hpp"
#include "Random.hpp"
#include "EnvModelView.hpp"

class EnvModel
{
public:
    struct Cell {
        float value;
        float time;
    };

    EnvModel(unsigned int mw, unsigned int mh, unsigned int ww, unsigned int wh, unsigned int n_layers = 1);

    void updateAll(void);
    void updatelLayer(unsigned int l);
    void setLayerFunction(unsigned int l, std::function<void(Cell&)> f);
    void displayInView(unsigned int l);

    void setValueByWorldCoord(float t, float x, float y, float v, float r = 0.f, unsigned int layer = 0);
    float getValueByWorldCoord(float x, float y, unsigned int layer) const;
    float getValueByModelCoord(unsigned int x, unsigned int y, unsigned int layer) const;
    void addLayers(unsigned int nl);
    void removeLayer(unsigned int l_id);

    /* Getters: */
    unsigned int getModelWidth(void) const { return m_model_w; }
    unsigned int getModelHeight(void) const { return m_model_h; }
    unsigned int getWorldWidth(void) const { return m_world_w; }
    unsigned int getWorldHeight(void) const { return m_world_h; }
    unsigned int getLayerCount(void) const { return m_num_layers; }
    const EnvModelView& getView(void) const { return m_view; }

protected:
    unsigned int m_model_w;
    unsigned int m_model_h;
    unsigned int m_world_w;
    unsigned int m_world_h;
    float m_ratio_w;            /* Unit width in world/pixels.  */
    float m_ratio_h;            /* Unit height in world/pixels. */
    unsigned int m_num_layers;
    std::map<int, std::vector<std::vector<Cell> > > m_model_cells;
    std::map<int, std::function<void(Cell&)> > m_layer_funcs;

    float getDistance(int x1, int y1, int x2, int y2) const;

private:
    EnvModelView m_view;
    int m_view_layer;
};

#endif /* ENV_MODEL_HPP */
