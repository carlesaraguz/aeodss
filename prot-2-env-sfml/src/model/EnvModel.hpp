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

class EnvModel
{
public:
    EnvModel(unsigned int mw, unsigned int mh, unsigned int ww, unsigned int wh, unsigned int n_layers = 1);
    ~EnvModel(void);

    void setValue(float x, float y, float v, float r = 0.f, unsigned int layer = 0);
    float getValue(float x, float y, unsigned int layer = 0) const;
    void addLayers(unsigned int nl);
    void removeLayer(unsigned int l_id);
    unsigned int getModelWidth(void) const { return m_model_w; }
    unsigned int getModelHeight(void) const { return m_model_h; }
    unsigned int getLayerCount(void) const { return m_num_layers; }

protected:
    struct EMCell {
        float value;
        float update_time;
    };
    unsigned int m_model_w;
    unsigned int m_model_h;
    unsigned int m_world_w;
    unsigned int m_world_h;
    float m_ratio_w;
    float m_ratio_h;
    unsigned int m_num_layers;
    std::map<int, EMCell**> m_model_cells;
};

#endif /* ENV_MODEL_HPP */
