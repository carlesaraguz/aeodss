#ifndef __TEST_BASICINSTRUMENT_HPP__
#define __TEST_BASICINSTRUMENT_HPP__

#include <gtest/gtest.h>

#include "BasicInstrument.hpp"
#include "prot.hpp"
#include "Init.hpp"
#include "MultiView.hpp"
#include "Agent.hpp"
#include "Config.hpp"
// CREATE_LOGGER(test_BasicInstrument)


namespace
{
    unsigned int n_agents = 1;
    bool play = true;

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

    TEST(BasicInstrumentTest, testBasicInstrument)
    {
        // Init::doInit();
        sf::ContextSettings settings;
        settings.antialiasingLevel = 0;
        sf::RenderWindow window(sf::VideoMode(Config::world_width, Config::world_height), "BasicInstrument Test Window", sf::Style::Titlebar | sf::Style::Close, settings);
        window.setFramerateLimit(60);

        /* Create agents: --------------------------------------------------------------------------- */
        std::vector<std::shared_ptr<Agent> > agents;
        for(unsigned int i = 0; i < n_agents; i++) {
            auto aptr = std::make_shared<Agent>("A" + std::to_string(i));
            agents.push_back(aptr);
        }

        /* Configure agents: ------------------------------------------------------------------------ */
        for(auto a : agents) {
            a->getLink()->setAgents(agents);
            a->displayActivities(ActivityDisplayType::ALL);
            a->showResources(true);
        }
        agents[0]->displayActivities(ActivityDisplayType::ALL);
        agents[0]->showResources(true);
        agents[0]->getEnvironment()->buildView();


        std::vector<std::shared_ptr<const HasView> > avs(agents.begin(), agents.end());   /* Casts. */
        MultiView mv1;
        mv1.setViews(avs);

        /* Event loop: -------------------------------------------------------------------------- */
        handleEvents(window);

        mv1.drawViews();

        window.clear();
        window.draw(mv1);

        /* Here, view enviornment is created and configured */
        /* For tests, we will suppose that apperture is a fixed value. */
        float aperture = 80.f;
        BasicInstrument *bi1 = new BasicInstrument(aperture);
        BasicInstrument *bi2 = new BasicInstrument(aperture);
        EXPECT_EQ(aperture, bi1->getAperture());
        EXPECT_EQ(aperture, bi2->getAperture());
        // BasicInstrument *bi3 = new BasicInstrument(aperture);

        /* Some critical positions must be ckecked in order to verify the proper behaviour
        * of our functions. The points will be (Writen in ECI coordinates):
        * (Same altitude in m in each point = R_E + 600000 =  6378137.0 + 600000.0 = 6978137.0)
        * NORTH POLE / SOUTH POLE:
        *  - (0, 0, 6978137.0) / (0, 0, -6978137.0)
        *
        * EQUATOR POINT (ON X AXIS):
        *  - (6978137.0, 0, 0)
        *
        * RANDOM POINT:
        *  - (,,);
        **/

        sf::Vector3f north_pole = sf::Vector3f(739445.f,2038920.f,5998930.f);
        sf::Vector3f equator    = sf::Vector3f(6978137.f, 1.f, 1.f);
        // sf::Vector3f random     = sf::Vector3f();

        bi1->setPosition(north_pole);
        // std::cout << "X: " << bi1->getProjection2D().x << "Y: " << bi1->getProjection2D() <<  '\n';
        bi2->setPosition(equator);
        // std::cout << "X: " << bi2->getProjection2D().x << "Y: " << bi2->getProjection2D() <<  '\n';

        ASSERT_EQ(bi1->getPosition(), north_pole);
        ASSERT_EQ(bi2->getPosition(), equator);

        std::vector<sf::Vector2f> fp_1 = bi1->getFootprint();
        std::vector<sf::Vector2f> fp_2 = bi2->getFootprint();

        std::cout << "FP1 size: " << fp_1.size() << '\n';
        std::cout << "FP2 size: " << fp_2.size() << '\n';

        // std::cout << '\n'<< "NORTH POLE FP:" << '\n';
        // for(auto& fp_point: fp_1) {
        //     std::cout << "X: " << fp_point.x << "Y: "<< fp_point.y << '\n';
        // }

        // std::cout << '\n' << "EQUATOR FP:" << '\n';
        // for(auto& fp_point: fp_2) {
        //     std::cout << "X: " << fp_point.x << " | Y: "<< fp_point.y << '\n';
        // }


        std::vector<ThickLine> footprint;
        // std::cout << "Size " << fp_1.size() << '\n';

        for(int i = 1; i < (int)fp_1.size(); i++) {
            ThickLine tl(fp_1[i - 1], fp_1[i]);
            tl.setThickness(2.f);
            // tl.setColor(Config::color_dark_green);
            tl.setColor(sf::Color::Cyan);
            // tl.setColor(sf::Color::White);
            footprint.push_back(tl);
        }
        for(auto& f_line : footprint) {
            window.draw(f_line);
        }

        // mv1.setViews(footprint);

        // sf::ConvexShape triangle1, triangle2, triangle3;
        // triangle1.setPoint(0, sf::Vector2f(Config::agent_size, 0.f));
        // triangle1.setPoint(2, sf::Vector2f(
        //     Config::agent_size * std::cos(-2.f * Config::pi / 3.0),
        //     Config::agent_size * std::sin(-2.f * Config::pi / 3.0) * 0.5f
        // ));
        // triangle1.setPoint(1, sf::Vector2f(
        //     Config::agent_size * std::cos(2.0 * Config::pi / 3.0),
        //     Config::agent_size * std::sin(2.0 * Config::pi / 3.0) * 0.5f
        // ));
        // triangle1.setFillColor(sf::Color::Cyan);
        // triangle1.setOutlineColor(sf::Color::Transparent);
        //
        // window.draw(triangle1);

        footprint.clear();
        for(int i = 1; i < (int)fp_2.size(); i++) {
            ThickLine tl(fp_2[i - 1], fp_2[i]);
            tl.setThickness(2.f);
            // tl.setColor(Config::color_dark_green);
            tl.setColor(sf::Color::Cyan);
            // tl.setColor(sf::Color::White);
            footprint.push_back(tl);
        }

        for(auto& f_line : footprint) {
            window.draw(f_line);
        }

        // triangle2.setPoint(0, sf::Vector2f(Config::agent_size, 0.f));
        // triangle2.setPoint(2, sf::Vector2f(
        //     Config::agent_size * std::cos(-2.f * Config::pi / 3.0),
        //     Config::agent_size * std::sin(-2.f * Config::pi / 3.0) * 0.5f
        // ));
        // triangle2.setPoint(1, sf::Vector2f(
        //     Config::agent_size * std::cos(2.0 * Config::pi / 3.0),
        //     Config::agent_size * std::sin(2.0 * Config::pi / 3.0) * 0.5f
        // ));
        // triangle2.setFillColor(sf::Color::Cyan);
        // triangle2.setOutlineColor(sf::Color::Transparent);
        //
        // window.draw(triangle2);

        window.display();
        sf::Image screen = window.capture();
        screen.saveToFile("../test_screenshot.jpg");
    }
}

#endif /* __TEST_BASICINSTRUMENT_HPP__*/
