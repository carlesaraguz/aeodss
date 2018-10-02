/***********************************************************************************************//**
 *  Estimated state of the environment that the Agents capture and get payoff from.
 *  @class      EnvModel
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-10
 *  @version    0.2
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef ENV_MODEL_HPP
#define ENV_MODEL_HPP

#include "prot.hpp"
#include "hashers.hpp"
#include <unordered_set>
#include "World.hpp"
#include "GridView.hpp"
#include "EnvCell.hpp"
#include "PayoffFunctions.hpp"
#include "HasView.hpp"

class Agent;

struct EnvModelInfo {
    unsigned int mh;     /* Cover a number of pixels. */
    unsigned int mw;     /* Cover a number of pixels. */
    float rw;            /* Unit width in [model-units / world-pixel].  */
    float rh;            /* Unit height in [model-units / world-pixel]. */
};

class EnvModel : public HasView
{
public:
    EnvModel(Agent* aptr, unsigned int mw, unsigned int mh);

    EnvModelInfo getEnvModelInfo(void) const { return { m_model_h, m_model_w, m_ratio_w, m_ratio_h }; }
    unsigned int getModelWidth(void) const { return m_model_w; }
    unsigned int getModelHeight(void) const { return m_model_h; }

    std::vector<sf::Vector2i> getWorldCells(EnvCell model_cell) const;
    std::vector<sf::Vector2i> getWorldCells(unsigned int x, unsigned int y) const;
    std::vector<sf::Vector2i> getWorldCells(sf::Vector2i model_cell) const;
    std::vector<sf::Vector2i> getWorldCells(std::vector<sf::Vector2i> model_cells) const;
    template <class T>
    sf::Vector2<T> toWorldCoordinates(sf::Vector2<T> model_coord) const;
    template <class T>
    sf::Vector2<T> toModelCoordinates(sf::Vector2<T> world_coord) const;
    void updateView(void);
    void clearView(void);
    void enableViewUnits(std::vector<sf::Vector2i> us);
    const GridView& getView(void) const;

private:
    Agent* m_agent;             /* Owner. */
    unsigned int m_model_h;     /* Cover a number of pixels. */
    unsigned int m_model_w;     /* Cover a number of pixels. */
    unsigned int m_world_h;     /* Real world magnitude (here in pixels.) */
    unsigned int m_world_w;     /* Real world magnitude (here in pixels.) */
    float m_ratio_w;            /* Unit width in [world-pixel / model-unit].  */
    float m_ratio_h;            /* Unit height in [world-pixel / model-unit]. */
    std::vector<std::vector<EnvCell> > m_cells;
    std::shared_ptr<GridView> m_view_payoff;
};

template <class T>
sf::Vector2<T> EnvModel::toWorldCoordinates(sf::Vector2<T> model_coord) const
{
    return sf::Vector2<T>(model_coord.x * m_ratio_w, model_coord.y * m_ratio_h);
}

template <class T>
sf::Vector2<T> EnvModel::toModelCoordinates(sf::Vector2<T> world_coord) const
{
    return sf::Vector2<T>(world_coord.x / m_ratio_w, world_coord.y / m_ratio_h);
}


#endif /* ENV_MODEL_HPP */
