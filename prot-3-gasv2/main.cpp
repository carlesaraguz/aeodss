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

int main(int argc, char** argv)
{
    Init::doInit();
    Config::loadCmdArgs(argc, argv);

    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;
    sf::RenderWindow window(sf::VideoMode(Config::win_width, Config::win_height), "Autonomous Nano-Satellite Swarm", sf::Style::Titlebar | sf::Style::Close, settings);
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
    // agents.push_back(std::make_shared<Agent>("A0", sf::Vector2f(0.f, 0.f), sf::Vector2f(10.f, 10.f)));
    // agents.push_back(std::make_shared<Agent>("A1", sf::Vector2f(0.f, 460.f), sf::Vector2f(10.f, -5.1f)));
    // agents.push_back(std::make_shared<Agent>("A2", sf::Vector2f(0.f, 460.f), sf::Vector2f(10.f, 0.f)));

    /* Configure agents: ------------------------------------------------------------------------ */
    for(auto a : agents) {
        a->getLink()->setAgents(agents);
        a->displayActivities(ActivityDisplayType::ALL);
        a->showResources(true);
    }
    agents[0]->displayActivities(ActivityDisplayType::ALL);
    agents[0]->showResources(true);
    agents[0]->getEnvironment()->buildView();

    /* Create a global aggregated environment model: -------------------------------------------- */
    auto world = std::make_shared<World>();
    world->addAgent(agents);

    /* Create multi-views: ---------------------------------------------------------------------- */
    std::vector<std::shared_ptr<const HasView> > avs(agents.begin(), agents.end());   /* Casts. */
    MultiView mv1, mv2, mv3, mv4;
    mv1.setViews(avs);
    mv2.addViewToBack(agents[0]->getEnvironment());
    mv2.addViewToBack(std::static_pointer_cast<const HasView>(agents[0]->getActivityHandler()));
    mv2.addViewToBack(avs[0]);
    mv3.addViewToBack(world);
    mv4.addViewToBack(world);
    for(unsigned int i = 0; i < agents.size(); i++) {
        mv1.addViewToBack(std::static_pointer_cast<const HasView>(agents[i]->getActivityHandler()));
        mv3.addViewToBack(avs[i]);
        mv4.addViewToBack(avs[i]);
    }

    mv1.setScale(0.5f, 0.5f);
    mv2.setScale(0.5f, 0.5f);
    mv3.setScale(0.5f, 0.5f);
    mv4.setScale(0.5f, 0.5f);
    mv2.setPosition(Config::win_width / 2.f, 0.f);
    mv3.setPosition(0.f, Config::win_height / 2.f);
    mv4.setPosition(Config::win_width / 2.f, Config::win_height / 2.f);

    // std::vector<std::shared_ptr<const HasView> > avs(agents.begin(), agents.end());   /* Casts. */
    // MultiView mv_world;
    // mv_world.addViewToBack(world);
    // for(unsigned int i = 0; i < agents.size(); i++) {
    //     // mv_world.addViewToBack(std::static_pointer_cast<const HasView>(agents[i]->getActivityHandler()));
    //     mv_world.addViewToBack(avs[i]);
    // }

    /* Add some background: */
    sf::Texture world_map_texture;
    sf::Sprite world_map1, world_map2, world_map3, world_map4;
    if(!world_map_texture.loadFromFile(Config::root_path + "res/equirectangular_map.png")) {
        Log::err << "Failed to load equirectangular map texture.\n";
        world_map1.setColor(sf::Color(0, 0, 0, 0));
        world_map2.setColor(sf::Color(0, 0, 0, 0));
        world_map3.setColor(sf::Color(0, 0, 0, 0));
        world_map4.setColor(sf::Color(0, 0, 0, 0));
    } else {
        world_map_texture.setSmooth(true);
        float scale_factor = (float)(Config::win_width / 2) / (float)world_map_texture.getSize().x;
        world_map1.setTexture(world_map_texture);
        world_map2.setTexture(world_map_texture);
        world_map3.setTexture(world_map_texture);
        world_map4.setTexture(world_map_texture);
        sf::Color omod = Config::color_orange;
        omod.a = 127;
        world_map1.setColor(omod); /* Half transparent orange. */
        world_map2.setColor(sf::Color(0, 0, 0, 128)); /* Half transparent gray. */
        world_map3.setColor(sf::Color(0, 0, 0, 128)); /* Half transparent gray. */
        world_map4.setColor(sf::Color(0, 0, 0, 128)); /* Half transparent gray. */
        world_map1.setScale(scale_factor, scale_factor);
        world_map2.setScale(scale_factor, scale_factor);
        world_map3.setScale(scale_factor, scale_factor);
        world_map4.setScale(scale_factor, scale_factor);
        world_map2.setPosition(Config::win_width / 2.f, 0.f);
        world_map3.setPosition(0.f, Config::win_height / 2.f);
        world_map4.setPosition(Config::win_width / 2.f, Config::win_height / 2.f);
    }

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
        // window.draw(mv_world);
        window.draw(mv1);
        window.draw(mv2);
        window.draw(mv3);
        window.draw(mv4);
        window.draw(world_map1);
        window.draw(world_map2);
        window.draw(world_map3);
        window.draw(world_map4);
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
