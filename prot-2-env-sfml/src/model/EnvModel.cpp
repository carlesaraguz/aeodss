/***********************************************************************************************//**
 *  The modeled environment.
 *  @class      EnvModel
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-28
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "EnvModel.hpp"

EnvModel::EnvModel(unsigned int mw, unsigned int mh, unsigned int ww, unsigned int wh, unsigned int n_layers)
    : m_world_w(ww)
    , m_world_h(wh)
    , m_model_w(mw)
    , m_model_h(mh)
    , m_num_layers(n_layers)
{
    if(m_model_w == 0 || m_model_h == 0) {
        std::cerr << "Environment model can't have 0 dimensions (" << m_model_w << ", " << m_model_h << ").\n";
        m_ratio_w = 1.f;
        m_ratio_h = 1.f;
        m_model_w = m_world_w;
        m_model_h = m_world_h;
    } else if(m_model_w > m_world_w || m_model_h > m_world_h) {
        std::cerr << "Environment model can't be bigger than the world (" << m_model_w << ", " << m_model_h << ").\n";
        m_ratio_w = 1.f;
        m_ratio_h = 1.f;
        m_model_w = m_world_w;
        m_model_h = m_world_h;
    } else {
        m_ratio_w = (float)m_world_w / (float)m_model_w;
        m_ratio_h = (float)m_world_h / (float)m_model_h;
    }
    for(unsigned int i = 0; i < m_num_layers; i++) {
        m_model_cells[i] = new EMCell*[m_model_w];
        for(unsigned int j = 0; j < m_model_w; j++) {
            m_model_cells[i][j] = new EMCell[m_model_h];
            for(unsigned int k = 0; k < m_model_h; k++) {
                m_model_cells[i][j][k].value = Random::getUf(0.f, 127.f);
                m_model_cells[i][j][k].update_time = -1.f;
            }
        }
    }
}

EnvModel::~EnvModel(void)
{
    for(auto& l : m_model_cells) {
        for(unsigned int i = 0; i < m_model_w; i++) {
            delete[] l.second[i];
        }
        delete[] l.second;
    }
}

void EnvModel::setValue(float x, float y, float v, float r, unsigned int layer)
{
    if(x > m_world_w || y > m_world_h) {
        std::cerr << "Environment model error. Setting a value out of world boundaries.\n";
        return;
    }
    if(layer >= m_num_layers) {
        std::cerr << "Environment model error. Setting a value in wrong layer " << layer << ".\n";
        return;
    }
    int ox = std::ceil(x / m_ratio_w);
    int oy = std::ceil(y / m_ratio_h);

    m_model_cells[layer][ox][oy].value = v;
    if(r > 0.f) {
        /*  Find other cells that are at a distance of `r` from (ox, oy):
         *    - ox, oy : Spiral origin/offset.
         *    - xx, yy : Spiral iterators (origin = 0,0).
         *    - dx, dy : Iterator steps.
         */
        int xx, yy, dx, dy;
        xx = yy = dx = 0.f;
        dy = -1;
        int t = 2 * std::max(m_model_w, m_model_h);
        int max_iter = t * t;
        bool at_r = false;
        int corner_count = 0;
        for(int i = 0; i < max_iter; i++) {
            if((xx + ox < m_model_w) && (xx + ox >= 0) && (yy + oy < m_model_h) && (yy + oy >= 0)) {
                if(std::abs(std::complex<float>(xx - ox, yy - oy)) <= r) {
                    m_model_cells[layer][xx + ox][yy + oy].value = v;
                    at_r = true;
                }
                if(corner_count >= 5 && !at_r) {
                    break;
                }
            }
            if((xx == yy) || ((xx < 0) && (xx == -yy)) || ((xx > 0) && (xx == 1 - yy))) {
                t  = dx;
                dx = -dy;
                dy = t;
                corner_count++;
            }
            xx += dx;
            yy += dy;
        }
    }

}

float EnvModel::getValue(float x, float y, unsigned int layer) const
{
    if(x > m_world_w || y > m_world_h) {
        std::cerr << "Environment model error. Requesting a value out of world boundaries.\n";
        return 0.f;
    }
    if(layer >= m_num_layers) {
        std::cerr << "Environment model error. Requesting a value in wrong layer " << layer << ".\n";
        return 0.f;
    }
    int ox = std::ceil(x / m_ratio_w);
    int oy = std::ceil(y / m_ratio_h);
    float v = m_model_cells.at(layer)[ox][oy].value;
    return m_model_cells.at(layer)[ox][oy].value;
}

void EnvModel::addLayers(unsigned int nl)
{
    /* TODO */
}

void EnvModel::removeLayer(unsigned int l_id)
{
    /* TODO */
}
