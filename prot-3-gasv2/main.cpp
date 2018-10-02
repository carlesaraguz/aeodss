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
#include "MultiView.hpp"
#include "Init.hpp"

CREATE_LOGGER(main)

void handleEvents(sf::RenderWindow& w);

bool play = true;

int main(int /* argc */, char** /* argv */)
{
    Init::doInit();

    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;
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
    // agents.push_back(std::make_shared<Agent>("A0", sf::Vector2f(100.f, 400.f), sf::Vector2f(10.f, 0.f)));
    // agents.push_back(std::make_shared<Agent>("A1", sf::Vector2f(300.f, 400.f), sf::Vector2f(-10.f, 0.f)));

    /* Configure agents: ------------------------------------------------------------------------ */
    for(auto a : agents) {
        a->getLink()->setAgents(agents);
    }
    agents[0]->displayActivities(ActivityDisplayType::ALL);
    agents[0]->showResources(true);

    /* Create a global aggregated environment model: -------------------------------------------- */
    auto world = std::make_shared<World>();
    world->addAgent(agents);

    /* Create multi-views: ---------------------------------------------------------------------- */
    std::vector<std::shared_ptr<const HasView> > avs(agents.begin(), agents.end());   /* Casts. */
    MultiView mv1, mv2, mv3, mv4;
    mv1.setViews(avs);
    mv2.addViewToBack(std::static_pointer_cast<const HasView>(agents[0]->getActivityHandler()));
    mv2.addViewToBack(avs[0]);
    mv3.addViewToBack(world);
    for(auto& av : avs) {
        mv3.addViewToBack(av);
    }
    mv4.addViewToBack(world);

    mv1.setScale(0.5f, 0.5f);
    mv2.setScale(0.5f, 0.5f);
    mv3.setScale(0.5f, 0.5f);
    mv4.setScale(0.5f, 0.5f);
    mv2.setPosition(Config::win_width / 2.f, 0.f);
    mv3.setPosition(0.f, Config::win_height / 2.f);
    mv4.setPosition(Config::win_width / 2.f, Config::win_height / 2.f);

    int draw_it = 0;

    while(window.isOpen()) {
        /* Event loop: -------------------------------------------------------------------------- */
        handleEvents(window);

        /* Update loop: ------------------------------------------------------------------------- */
        std::vector<std::thread> thread_pool;
        if(play) {
            VirtualTime::step();
            /* Update agents: */
            for(auto& a : agents) {
                a->step();
                a->getLink()->update();
                a->getLink()->step();
            }
            world->step();
        }

        /* Pre-draw loop: ----------------------------------------------------------------------- */
        mv1.drawViews();
        mv2.drawViews();
        if(draw_it % 10 == 0) {
            draw_it = 0;
            world->display(World::Layer::REVISIT_TIME_ACTUAL);
            mv3.drawViews();
            world->display(World::Layer::REVISIT_TIME_BEST);
            mv4.drawViews();
        }

        /* Draw loop: --------------------------------------------------------------------------- */
        window.clear();
        window.draw(mv1);
        window.draw(mv2);
        window.draw(mv3);
        window.draw(mv4);
        window.display();
        draw_it++;
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
            if(event.key.code == sf::Keyboard::Space) {
                if(play) {
                    std::cout << "Stopping world.\n";
                    play = false;
                } else {
                    std::cout << "Resuming world.\n";
                    play = true;
                }
            }
        }
    }
}
