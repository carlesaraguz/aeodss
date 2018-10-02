/***********************************************************************************************//**
 *  An aggregation of several environment models.
 *  @class      GlobalEnvModel
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-28
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "GlobalEnvModel.hpp"

GlobalEnvModel::GlobalEnvModel(std::array<unsigned int, 5> dim)
    : GlobalEnvModel(dim[0], dim[1], dim[2], dim[3], dim[4])
{ }

GlobalEnvModel::GlobalEnvModel(unsigned int mw, unsigned int mh, unsigned int ww, unsigned int wh, unsigned int n_layers)
    : m_world_w(ww)
    , m_world_h(wh)
    , m_model_w(mw)
    , m_model_h(mh)
    , m_num_layers(n_layers)
    , m_view(mw, mh, ww, wh)
    , m_view_layer(-1)
{
    m_view.show();
    if(m_model_w == 0 || m_model_h == 0) {
        std::cerr << "Global Environment model can't have 0 dimensions (" << m_model_w << ", " << m_model_h << ").\n";
        m_ratio_w = 1.f;
        m_ratio_h = 1.f;
        m_model_w = m_world_w;
        m_model_h = m_world_h;
    } else if(m_model_w > m_world_w || m_model_h > m_world_h) {
        std::cerr << "Global Environment model can't be bigger than the world (" << m_model_w << ", " << m_model_h << ").\n";
        m_ratio_w = 1.f;
        m_ratio_h = 1.f;
        m_model_w = m_world_w;
        m_model_h = m_world_h;
    } else {
        m_ratio_w = (float)m_world_w / (float)m_model_w;
        m_ratio_h = (float)m_world_h / (float)m_model_h;
    }
    for(unsigned int i = 0; i < m_num_layers; i++) {
        std::vector<std::vector<float> > css;
        css.reserve(m_model_w);
        for(unsigned int j = 0; j < m_model_w; j++) {
            std::vector<float> cs(m_model_h, 0.f);
            css.push_back(cs);
        }
        m_model_cells[i] = css;
    }
}

void GlobalEnvModel::displayInView(unsigned int l)
{
    if(l >= m_num_layers) {
        std::cerr << "Global Environment model error. Trying to display wrong layer " << l << ".\n";
        return;
    }
    m_view_layer = l;
    m_view.display(*this, m_view_layer);
}

void GlobalEnvModel::updateAll(void)
{
    for(auto& l : m_model_cells) {
        updatelLayer(l.first);
    }
}

void GlobalEnvModel::updatelLayer(unsigned int l)
{
    if(l >= m_num_layers) {
        std::cerr << "Global Environment model error. Trying to update a wrong layer " << l << ".\n";
        return;
    }
    for(std::size_t x = 0; x < m_model_cells[l].size(); x++) {
        for(std::size_t y = 0; y < m_model_cells[l][x].size(); y++) {
            m_model_cells[l][x][y] = 0.f;
            for(std::size_t e = 0; e < m_em_set.size(); e++) {
                m_model_cells[l][x][y] = std::max(m_model_cells[l][x][y], m_em_set[e]->getValueByModelCoord(x, y, l));
            }
        }
    }
    if(m_view_layer == (int)l) {
        m_view.display(*this, m_view_layer);
    }
}

void GlobalEnvModel::addEnvModel(const EnvModel* e)
{
    m_em_set.push_back(e);
}

float GlobalEnvModel::getValueByModelCoord(unsigned int x, unsigned int y, unsigned int layer) const
{
    if(x > m_model_w || y > m_model_h) {
        std::cerr << "Global Environment model error. Requesting a value out of world boundaries.\n";
        return 0.f;
    }
    if(layer >= m_num_layers) {
        std::cerr << "Global Environment model error. Requesting a value in wrong layer " << layer << ".\n";
        return 0.f;
    }
    return m_model_cells.at(layer)[x][y];
}

EnvModelView& GlobalEnvModel::getView(void)
{
    if(m_view.isHidden()) {
        m_view.show();
    }
    return m_view;
}
