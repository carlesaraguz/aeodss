/***********************************************************************************************//**
 *  An aggregation of several environment models.
 *  @class      GlobalEnvModel
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-28
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef GLOBAL_ENV_MODEL_HPP
#define GLOBAL_ENV_MODEL_HPP

#include "prot.hpp"
#include "EnvModel.hpp"
#include "EnvModelView.hpp"

class GlobalEnvModel
{
public:
    GlobalEnvModel(std::array<unsigned int, 5> dim);
    GlobalEnvModel(unsigned int mw, unsigned int mh, unsigned int ww, unsigned int wh, unsigned int n_layers = 1);

    void displayInView(unsigned int l);
    void updateAll(void);
    void updatelLayer(unsigned int l);
    void addEnvModel(const EnvModel* e);

    float getValueByModelCoord(unsigned int x, unsigned int y, unsigned int layer) const;

    EnvModelView& getView(void);

private:
    std::vector<const EnvModel*> m_em_set;
    std::map<int, std::vector<std::vector<float> > > m_model_cells;

    unsigned int m_model_w;
    unsigned int m_model_h;
    unsigned int m_world_w;
    unsigned int m_world_h;
    float m_ratio_w;
    float m_ratio_h;
    unsigned int m_num_layers;
    int m_view_layer;

    float m_max_value;
    float m_min_value;
    float m_mean_value;

    EnvModelView m_view;
};

#endif /* GLOBAL_ENV_MODEL_HPP */
