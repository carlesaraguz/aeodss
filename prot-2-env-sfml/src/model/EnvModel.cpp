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
    , m_view(mw, mh, ww, wh)
    , m_view_layer(-1)
{
    m_view.hide();
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
    int cid_acc = 0;
    for(unsigned int i = 0; i < m_num_layers; i++) {
        std::vector<std::vector<Cell> > css;
        css.reserve(m_model_w);
        for(unsigned int j = 0; j < m_model_w; j++) {
            std::vector<Cell> cs;
            cs.reserve(m_model_h);
            for(unsigned int k = 0; k < m_model_h; k++) {
                Cell c = {
                    (j > m_model_w / 2.f ? -1.f : 1.f),       /* Initial .value       */
                    (j > m_model_w / 2.f ? -1.f : 0.f),       /* Initial .update_time */
                    cid_acc++   /* Cell ID: .cid        */
                };
                cs.push_back(c);
            }
            css.push_back(cs);
        }
        m_model_cells[i] = css;
        m_layer_funcs[i] = [](Cell&) { };   /* Function that does nothing. */
    }
}

void EnvModel::updateAll(void)
{
    for(auto& l : m_model_cells) {
        updatelLayer(l.first);
    }
}

void EnvModel::displayInView(unsigned int l)
{
    if(l >= m_num_layers) {
        std::cerr << "Environment model error. Trying to display wrong layer " << l << ".\n";
        return;
    }
    m_view_layer = l;
    m_view.display(*this, m_view_layer);
}

void EnvModel::updatelLayer(unsigned int l)
{
    if(l >= m_num_layers) {
        std::cerr << "Environment model error. Trying to update a wrong layer " << l << ".\n";
        return;
    }
    for(std::size_t x = 0; x < m_model_cells[l].size(); x++) {
        for(std::size_t y = 0; y < m_model_cells[l][x].size(); y++) {
            // if(m_model_cells[l][x][y].time != -1.f) {
                m_layer_funcs[l](m_model_cells[l][x][y]);
            // }
        }
    }
    if(m_view_layer == (int)l) {
        m_view.display(*this, m_view_layer);
    }
}

void EnvModel::setLayerFunction(unsigned int l, std::function<void(Cell&)> f)
{
    if(l >= m_num_layers) {
        std::cerr << "Environment model error. Setting a function layer in wrong layer " << l << ".\n";
    } else {
        m_layer_funcs[l] = f;
    }
}

void EnvModel::setValueByWorldCoord(float t, float x, float y, float v, float r, unsigned int layer, bool update_after)
{
    if(x > m_world_w || y > m_world_h) {
        std::cerr << "Environment model error. Setting a value out of world boundaries.\n";
        return;
    }
    if(layer >= m_num_layers) {
        std::cerr << "Environment model error. Setting a value in wrong layer " << layer << ".\n";
        return;
    }
    int ox = std::round((x - (m_ratio_w / 2.f)) / m_ratio_w);
    int oy = std::round((y - (m_ratio_h / 2.f)) / m_ratio_h);

    m_model_cells[layer][ox][oy].value = v;
    m_model_cells[layer][ox][oy].time  = t;
    if(r > 0.f) {
        /*  Find other cells that are at a distance of `r` from (ox, oy):
         *    - ox, oy : Spiral origin/offset.
         *    - xx, yy : Spiral iterators (origin = 0,0).
         *    - dx, dy : Iterator steps.
         */
        int xx, yy, dx, dy;
        xx = yy = dx = 0.f;
        dy = -1;
        int b = 2 * std::max(r / m_ratio_w, r / m_ratio_h);
        int max_iter = b * b;
        bool at_r = false;
        int corner_count = 0;
        for(int i = 0; i < max_iter; i++) {
            if((xx + ox < (int)m_model_w) && (xx + ox >= 0) && (yy + oy < (int)m_model_h) && (yy + oy >= 0)) {
                float dist = getDistance(ox, oy, ox + xx, oy + yy);
                if(dist <= r) {
                    m_model_cells[layer][xx + ox][yy + oy].value = v;
                    m_model_cells[layer][xx + ox][yy + oy].time  = t;
                    if(update_after) {
                        m_layer_funcs[layer](m_model_cells[layer][xx + ox][yy + oy]);
                    }
                    at_r = true;
                }
                if(corner_count >= 5 && !at_r) {
                    break;
                }
            }
            if((xx == yy) || ((xx < 0) && (xx == -yy)) || ((xx > 0) && (xx == 1 - yy))) {
                b  = dx;
                dx = -dy;
                dy = b;
                corner_count++;
            }
            xx += dx;
            yy += dy;
        }
    }
}

float EnvModel::getDistance(int x1, int y1, int x2, int y2) const
{
    float x = (x1 - x2) * m_ratio_w;
    float y = (y1 - y2) * m_ratio_h;
    return std::sqrt(x * x + y * y);
}

float EnvModel::getValueByWorldCoord(float x, float y, float r, unsigned int layer, Aggregate fn) const
{
    if(x > m_world_w || y > m_world_h) {
        std::cerr << "Environment model error. Requesting a value out of world boundaries (x = " << x << ", y = " << y << ").\n";
        return 0.f;
    }
    if(layer >= m_num_layers) {
        std::cerr << "Environment model error. Requesting a value in wrong layer " << layer << ".\n";
        return 0.f;
    }
    int ox = std::round((x - (m_ratio_w / 2.f)) / m_ratio_w);
    int oy = std::round((y - (m_ratio_h / 2.f)) / m_ratio_h);

    float retval = m_model_cells.at(layer)[ox][oy].value;
    float mean_count = 0.f;
    if(r > 0.f) {
        /*  Find other cells that are at a distance of `r` from (ox, oy):
         *    - ox, oy : Spiral origin/offset.
         *    - xx, yy : Spiral iterators (origin = 0,0).
         *    - dx, dy : Iterator steps.
         */
        int xx, yy, dx, dy;
        xx = yy = dx = 0.f;
        dy = -1;
        int b = 2 * std::max(r / m_ratio_w, r / m_ratio_h);
        int max_iter = b * b;
        bool at_r = false;
        int corner_count = 0;
        for(int i = 0; i < max_iter; i++) {
            if((xx + ox < (int)m_model_w) && (xx + ox >= 0) && (yy + oy < (int)m_model_h) && (yy + oy >= 0)) {
                float dist = getDistance(ox, oy, ox + xx, oy + yy);
                if(dist <= r) {
                    switch(fn) {
                        case Aggregate::MAX_VALUE:
                            retval = std::max(retval, m_model_cells.at(layer)[xx + ox][yy + oy].value);
                            break;
                        case Aggregate::MIN_VALUE:
                            retval = std::min(retval, m_model_cells.at(layer)[xx + ox][yy + oy].value);
                            break;
                        case Aggregate::MEAN_VALUE:
                            retval += m_model_cells.at(layer)[xx + ox][yy + oy].value;
                            mean_count++;
                            break;
                    }
                    at_r = true;
                }
                if(corner_count >= 5 && !at_r) {
                    break;
                }
            }
            if((xx == yy) || ((xx < 0) && (xx == -yy)) || ((xx > 0) && (xx == 1 - yy))) {
                b  = dx;
                dx = -dy;
                dy = b;
                corner_count++;
            }
            xx += dx;
            yy += dy;
        }
    }
    if(fn == Aggregate::MEAN_VALUE) {
        retval /= mean_count;
    }
    return retval;
}

float EnvModel::getValueByModelCoord(unsigned int x, unsigned int y, unsigned int layer) const
{
    if(x > m_model_w || y > m_model_h) {
        std::cerr << "Environment model error. Requesting a value out of world boundaries.\n";
        return 0.f;
    }
    if(layer >= m_num_layers) {
        std::cerr << "Environment model error. Requesting a value in wrong layer " << layer << ".\n";
        return 0.f;
    }
    return m_model_cells.at(layer)[x][y].value;
}

EnvModelView& EnvModel::getView(void)
{
    if(m_view.isHidden()) {
        m_view.show();
    }
    return m_view;
}


void EnvModel::addLayers(unsigned int /* nl */)
{
    /* TODO */
}

void EnvModel::removeLayer(unsigned int /* l_id */)
{
    /* TODO */
}
