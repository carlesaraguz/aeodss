/***********************************************************************************************//**
 *  A wrapper for a rendering target of a number of agents.
 *  @class      WorldView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef WORLD_VIEW_HPP
#define WORLD_VIEW_HPP

#include "prot.hpp"
#include "Agent.hpp"
#include "GlobalEnvModel.hpp"

enum class WorldViewType {
    SINGLE_AGENT,   /**< Display the state and environment of a single agent. */
    GLOBAL_REAL,    /**< Display all agents, their plans and the data downloaded. */
    GLOBAL_ORACLE   /**< Display the actual state of the environment. */
};

class WorldView : public sf::Drawable, public sf::Transformable
{
public:
    WorldView(WorldViewType type, std::vector<std::shared_ptr<Agent> > agents, std::shared_ptr<GlobalEnvModel> global = nullptr);
    void drawWorld(void);

private:
    sf::RenderTexture m_target_texture;
    sf::Sprite m_canvas;
    sf::RectangleShape m_border;
    std::vector<std::shared_ptr<Agent> > m_agents;
    WorldViewType m_type;
    std::shared_ptr<GlobalEnvModel> m_oracle_model;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif /* WORLD_VIEW_HPP */
