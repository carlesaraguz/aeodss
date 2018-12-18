/***********************************************************************************************//**
 *  Estimated state of the environment that the Agents capture and get payoff from.
 *  @class      EnvModel
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-10
 *  @version    0.2
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "EnvModel.hpp"
#include "Agent.hpp"

CREATE_LOGGER(EnvModel)

EnvModel::EnvModel(Agent* aptr, unsigned int mw, unsigned int mh)
    : m_agent(aptr)
    , m_model_w(mw)
    , m_model_h(mh)
    , m_world_w(World::getWidth())
    , m_world_h(World::getHeight())
    , m_payoff_view(nullptr)
{
    if(m_model_w == 0 || m_model_h == 0) {
        Log::warn << "Model can't have 0-length dimensions (" << m_model_w << ", " << m_model_h << ").\n";
        m_ratio_w = 1.f;
        m_ratio_h = 1.f;
        m_model_w = m_world_w;
        m_model_h = m_world_h;
    } else if(m_model_w > m_world_w || m_model_h > m_world_h) {
        Log::warn << "Model can't be bigger than the world (" << m_model_w << ", " << m_model_h << ").\n";
        m_ratio_w = 1.f;
        m_ratio_h = 1.f;
        m_model_w = m_world_w;
        m_model_h = m_world_h;
    } else {
        m_ratio_w = (float)m_world_w / (float)m_model_w;
        m_ratio_h = (float)m_world_h / (float)m_model_h;
    }

    m_cells.reserve(m_model_w);
    if(Config::motion_model == AgentMotionType::ORBITAL) {
        m_world_positions.reserve(m_model_w);
    }
    float lat, lng;
    for(unsigned int i = 0; i < m_model_w; i++) {
        std::vector<EnvCell> column;
        std::vector<sf::Vector3f> column_lut;
        column.reserve(m_model_h);
        if(Config::motion_model == AgentMotionType::ORBITAL) {
            column_lut.reserve(m_model_h);
        }
        for(unsigned int j = 0; j < m_model_h; j++) {
            EnvCell c(i, j);
            c.pushPayoffFunc(PayoffFunctions::f_revisit_time_backwards);
            column.push_back(c);
            if(Config::motion_model == AgentMotionType::ORBITAL) {
                lng =   (360.f * (i * m_ratio_w) / World::getWidth()) - 180.f;
                lat = -((180.f * (j * m_ratio_h) / World::getHeight()) - 90.f);
                auto position_ecef = CoordinateSystemUtils::fromGeographicToECEF(sf::Vector3f(lat, lng, 0.f));
                column_lut.push_back(position_ecef);
            }
        }
        m_cells.push_back(column);
        if(Config::motion_model == AgentMotionType::ORBITAL) {
            m_world_positions.push_back(column_lut);
        }
    }
}

void EnvModel::buildView(void)
{
    m_payoff_view = std::make_shared<GridView>(m_model_w, m_model_h, Config::model_unity_size, Config::model_unity_size);
    m_payoff_view->setColorGradient(Config::color_gradient_krbg);
}

void EnvModel::clearView(void)
{
    if(m_payoff_view) {
        m_payoff_view->setValue(0.f);
    }
}

void EnvModel::computePayoff(std::shared_ptr<Activity> tmp_act, bool display_in_view)
{
    double* t0s;
    double* t1s;
    float po;
    if(display_in_view && m_payoff_view) {
        clearView();
    }
    int nts;

    auto cells = tmp_act->getActiveCells();
    #pragma omp parallel for
    for(std::size_t i = 0; i < cells.size(); i++) {
        auto& c = cells[i];
        nts = tmp_act->getCellTimes(c.x, c.y, &t0s, &t1s);
        po = m_cells[c.x][c.y].computeCellPayoff(0, t0s, t1s, nts);
        if(display_in_view && m_payoff_view) {
            m_payoff_view->setValue(c.x, c.y, po);
        }
    }
}

void EnvModel::addActivity(std::shared_ptr<Activity> act)
{
    auto cells = act->getActiveCells();
    #pragma omp parallel for
    for(std::size_t i = 0; i < cells.size(); i++) {
        auto& c = cells[i];
        m_cells[c.x][c.y].addCellActivity(act);
    }
}

bool EnvModel::removeActivity(std::shared_ptr<Activity> act)
{
    auto cells = act->getActiveCells();
    bool retval = false;
    #pragma omp parallel for
    for(std::size_t i = 0; i < cells.size(); i++) {
        auto& c = cells[i];
        retval |= m_cells[c.x][c.y].removeCellActivity(act);
    }
    return retval;
}

void EnvModel::cleanActivities(double t)
{
    if(t == -1.f) {
        t = VirtualTime::now();
    }
    #pragma omp parallel for
    for(unsigned int xx = 0; xx < m_model_w; xx++) {
        #pragma omp parallel for
        for(unsigned int yy = 0; yy < m_model_h; yy++) {
            m_cells[xx][yy].clean(0, t);
        }
    }
}

std::vector<ActivityGen> EnvModel::generateActivities(std::shared_ptr<Activity> tmp_act)
{
    double duration = tmp_act->getEndTime() - tmp_act->getStartTime();
    Log::dbg << "[" << m_agent->getId() << "] Generating potential activities in the range t = ["
        << VirtualTime::toString(tmp_act->getStartTime()) << ", "
        << VirtualTime::toString(tmp_act->getEndTime()) << "] ==> Duration: "
        << VirtualTime::toString(duration, false) << ".\n";
    std::vector<ActivityGen> retval;

    /* Iterate in time steps, to generate new activities: */
    double tstart = tmp_act->getStartTime();
    double tend = tmp_act->getEndTime();
    double t, t0 = 0.f, t1;
    bool bflag = false;
    std::unordered_set<sf::Vector2i, Vector2iHash> selected_cells;
    for(unsigned int s = 0; tstart + s * Config::time_step <= tend; s++) {
        t = tstart + s * Config::time_step;
        auto cells = tmp_act->getActiveCells(t);
        /* Remove meaningless cells (i.e. those that don't provide enough payoff at time `t`.) */
        for(auto it = cells.begin(); it != cells.end(); ) {
            EnvCell& c = m_cells[it->x][it->y];
            bool remove_cell = true;
            for(auto p : c.getAllPayoffs()) {
                if(p.second > Config::min_payoff) {
                    remove_cell = false;
                    break;
                }
            }
            if(remove_cell) {
                it = cells.erase(it);
            } else {
                selected_cells.insert(*it);
                it++;
            }
        }
        if(bflag && (cells.size() == 0 || (t - t0) >= Config::max_task_duration || t >= (tend - Config::time_step))) {
            /* End the previous activity: */
            t1 = t;
            bflag = false;
            if(t1 > t0 && selected_cells.size() > 0) {
                /*  DEBUG with:
                 *  Log::dbg << "[" << m_agent->getId() << "] Activity #" << retval.size() << ", T start = "
                 *      << std::setw(16) << VirtualTime::toString(t0) << ", end = " << std::setw(16) << VirtualTime::toString(t1)
                 *      << ", duration = " << VirtualTime::toString(t1 - t0, false) << ". Cell count: " << selected_cells.size() << std::defaultfloat << ".\n";
                 **/
                std::vector<sf::Vector2i> vec_selected_cells(selected_cells.begin(), selected_cells.end());
                std::vector<float> vec_payoffs;
                for(auto& vsc : vec_selected_cells) {
                    vec_payoffs.push_back(m_cells.at(vsc.x).at(vsc.y).getPayoff((t0 + t1) / 2.f));     /* COMBAK */
                }
                ActivityGen ag;
                ag.t0 = t0;
                ag.t1 = t1;
                ag.c_coord   = vec_selected_cells;
                ag.c_payoffs = vec_payoffs;
                retval.push_back(ag);
            }
            selected_cells.clear();
        }
        if(!bflag && cells.size() > 0) {
            /* Start a new activity: */
            t0 = t;
            bflag = true;
        }
        if(retval.size() >= Config::max_tasks) {
            break;
        }
    }
    return retval;
}

const GridView& EnvModel::getView(void) const
{
    if(m_payoff_view == nullptr) {
        Log::err << "Environment view for agent " << m_agent->getId() << " has not been initialized but has just been requested.\n";
        throw std::runtime_error("Uninitialized environment view requested");
    }
    return *m_payoff_view;
}

std::vector<sf::Vector2i> EnvModel::getWorldCells(EnvCell model_cell) const
{
    return getWorldCells(sf::Vector2i(model_cell.x, model_cell.y));
}

std::vector<sf::Vector2i> EnvModel::getWorldCells(unsigned int x, unsigned int y) const
{
    return getWorldCells(sf::Vector2i(x, y));
}

std::vector<sf::Vector2i> EnvModel::getWorldCells(sf::Vector2i model_cell) const
{
    std::vector<sf::Vector2i> retval;
    int length_hor = m_ratio_w;
    int length_ver = m_ratio_h;
    int start_x = ((float)model_cell.x * m_ratio_w);
    int start_y = ((float)model_cell.y * m_ratio_h);
    for(int xx = 0; xx < length_hor; xx++) {
        for(int yy = 0; yy < length_ver; yy++) {
            retval.push_back(sf::Vector2i(start_x + xx, start_y + yy));
        }
    }
    return retval;
}

std::vector<sf::Vector2i> EnvModel::getWorldCells(std::vector<sf::Vector2i> model_cells) const
{
    std::unordered_set<sf::Vector2i, Vector2iHash> total_world_cells;
    for(const auto& model_cell : model_cells) {
        auto world_cells = getWorldCells(model_cell);
        for(auto wc : world_cells) {
            total_world_cells.insert(wc);
        }
    }
    return std::vector<sf::Vector2i>(total_world_cells.begin(), total_world_cells.end());
}
