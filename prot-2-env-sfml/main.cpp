/***********************************************************************************************//**
 *  Program entry point.
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mar-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "prot.hpp"

#include "Agent.hpp"
#include "WorldView.hpp"
#include "Init.hpp"

void handleEvents(sf::RenderWindow& w);

int main(int /* argc */, char** /* argv */)
{
    Init::doInit();

    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;
    sf::RenderWindow window(sf::VideoMode(Config::win_width, Config::win_height), "Prototype", sf::Style::Default, settings);
    window.setFramerateLimit(60);
    /*  NOTE:
     *  Update mechanism should be changed to something user controlled and rely upon sf::Clock and
     *  elapsedTime() instead of just running uncontrolled.
     **/


    /* Create agents: --------------------------------------------------------------------------- */
    std::vector<std::shared_ptr<Agent> > agents;
    for(unsigned int i = 0; i < Config::n_agents; i++) {
        auto aptr = std::make_shared<Agent>("A" + std::to_string(i));
        agents.push_back(aptr);
    }

    /* Create world views: ---------------------------------------------------------------------- */
    WorldView wv1(WorldViewType::SINGLE_AGENT, std::vector<std::shared_ptr<Agent> >({ agents[0] }));
    WorldView wv2(WorldViewType::SINGLE_AGENT, std::vector<std::shared_ptr<Agent> >({ agents[1] }));
    WorldView wv3(WorldViewType::GLOBAL_REAL, agents);
    wv1.setScale(0.5f, 0.5f);
    wv2.setScale(0.5f, 0.5f);
    wv3.setScale(0.5f, 0.5f);
    wv2.setPosition(Config::win_width / 2.f, 0.f);
    wv3.setPosition(0.f, Config::win_height / 2.f);

    while(window.isOpen()) {
        /* Event loop: -------------------------------------------------------------------------- */
        handleEvents(window);

        /* Update loop: ------------------------------------------------------------------------- */
        for(auto& a : agents) {
            a->step();
        }

        wv1.drawWorld();
        wv2.drawWorld();
        wv3.drawWorld();

        /* Draw loop: --------------------------------------------------------------------------- */
        window.clear();
        window.draw(wv1);
        window.draw(wv2);
        window.draw(wv3);
        window.display();
    }

    return 0;
}

void handleEvents(sf::RenderWindow& w)
{
    sf::Event event;
    while(w.pollEvent(event)) {
        if(event.type == sf::Event::Closed) {
            w.close();
        }
        if(event.type == sf::Event::KeyPressed) {
            if(event.key.code == sf::Keyboard::Escape) {
                std::cout << "The escape key was pressed. Exiting." << std::endl;
                w.close();
            }
        }
    }
}
