#ifndef __TEST_BASICINSTRUMENT_HPP__
#define __TEST_BASICINSTRUMENT_HPP__

#include <gtest/gtest.h>

#include "BasicInstrument.hpp"
#include "prot.hpp"
#include "Init.hpp"
#include "MultiView.hpp"
#include "Agent.hpp"
#include "Config.hpp"
#include "Random.hpp"
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
        float aperture = 100.f;
        BasicInstrument *bi1 = new BasicInstrument(aperture);
        BasicInstrument *bi2 = new BasicInstrument(aperture);
        BasicInstrument *bi3 = new BasicInstrument(aperture);
        BasicInstrument *bi4 = new BasicInstrument(aperture);
        ASSERT_EQ(aperture, bi1->getAperture());
        ASSERT_EQ(aperture, bi2->getAperture());
        ASSERT_EQ(aperture, bi3->getAperture());
        ASSERT_EQ(aperture, bi4->getAperture());

        /* Some critical positions must be ckecked in order to verify the proper behaviour
        * of our functions. The points will be (Writen in LLA coordinates):
        * (Same altitude in m in each point = 600000m)
        * NORTH POLE / SOUTH POLE:
        *  - (0.f, 90.f, 600000) / (0.f, -90.f, 600000.f)
        *
        * EQUATOR POINT (ON X AXIS):
        *  - (0.f, 0.f, 600000.f)
        *
        * SPLITTED AND DEFORMED POINT:
        *  - (75.f, 165.f, 600000.f);
        * RANDOM POINT
        **/

        float lat = Random::getUf(-90.f, 90.f);
        float lon = Random::getUf(-180.f, 180.f);

        std::cout << "Lat: " << lat << "| Lon: " << lon << '\n';

        sf::Vector3f north_pole = sf::Vector3f(90.f, 0.f, 600000); /* In geographic coordinates */
        sf::Vector3f equator    = sf::Vector3f(0.f, 0.f, 600000);  /* In geographic coordinates */
        sf::Vector3f split      = sf::Vector3f(75.7067, 165.199, 600000.f); /* In geographic coordinates */
        sf::Vector3f random     = sf::Vector3f(lat, lon, 600000.f);

        sf::Vector3f north_pole_eci = CoordinateSystemUtils::fromGeographicToECI(north_pole, VirtualTime::now());
        sf::Vector3f equator_eci    = CoordinateSystemUtils::fromGeographicToECI(equator, VirtualTime::now());
        sf::Vector3f split_eci      = CoordinateSystemUtils::fromGeographicToECI(split, VirtualTime::now());
        sf::Vector3f random_eci     = CoordinateSystemUtils::fromGeographicToECI(random, VirtualTime::now());
        bi1->setPosition(north_pole_eci);
        bi2->setPosition(equator_eci);
        bi3->setPosition(split_eci);
        bi4->setPosition(random_eci);

        ASSERT_EQ(bi1->getPosition(), north_pole_eci);
        ASSERT_EQ(bi2->getPosition(), equator_eci);
        ASSERT_EQ(bi3->getPosition(), split_eci);
        ASSERT_EQ(bi4->getPosition(), random_eci);

        std::cout << "AGENT 1 POSITION:" << '\n';
        std::cout << "X: " << bi1->getPosition().x << " Y: " << bi1->getPosition().y << " Z: " << bi1->getPosition().z << '\n';
        std::cout << "AGENT 2 POSITION:" << '\n';
        std::cout << "X: " << bi2->getPosition().x << " Y: " << bi2->getPosition().y << " Z: " << bi2->getPosition().z << '\n';
        std::cout << "AGENT 3 POSITION:" << '\n';
        std::cout << "X: " << bi3->getPosition().x << " Y: " << bi3->getPosition().y << " Z: " << bi3->getPosition().z << '\n';
        std::cout << "AGENT 3 POSITION:" << '\n';
        std::cout << "X: " << bi4->getPosition().x << " Y: " << bi4->getPosition().y << " Z: " << bi4->getPosition().z << '\n';

        sf::Vector2f p_pos1 = AgentMotion::getProjection2D(bi1->getPosition(), VirtualTime::now());
        sf::Vector2f p_pos2 = AgentMotion::getProjection2D(bi2->getPosition(), VirtualTime::now());
        sf::Vector2f p_pos3 = AgentMotion::getProjection2D(bi3->getPosition(), VirtualTime::now());
        sf::Vector2f p_pos4 = AgentMotion::getProjection2D(bi4->getPosition(), VirtualTime::now());

        std::cout << "AGENT 1 PROJECTION:" << '\n';
        std::cout << "X: " << p_pos1.x << " Y: " << p_pos1.y << '\n';
        std::cout << "AGENT 2 PROJECTION:" << '\n';
        std::cout << "X: " << p_pos2.x << " Y: " << p_pos2.y << '\n';
        std::cout << "AGENT 3 PROJECTION:" << '\n';
        std::cout << "X: " << p_pos3.x << " Y: " << p_pos3.y << '\n';
        std::cout << "AGENT 4 PROJECTION:" << '\n';
        std::cout << "X: " << p_pos4.x << " Y: " << p_pos4.y << '\n';

        std::vector<sf::Vector2f> fp_1 = bi1->getFootprint();
        std::vector<sf::Vector2f> fp_2 = bi2->getFootprint();
        std::vector<sf::Vector2f> fp_3 = bi3->getFootprint();
        std::vector<sf::Vector2f> fp_4 = bi4->getFootprint();

        for(unsigned int i = 0; i < fp_1.size(); i++) {
            fp_1[i] += p_pos1;
        }

        for(unsigned int i = 0; i < fp_2.size(); i++) {
            fp_2[i] += p_pos2;
        }

        for(unsigned int i = 0; i < fp_3.size(); i++) {
            fp_3[i] += p_pos3;
        }

        for(unsigned int i = 0; i < fp_4.size(); i++) {
            fp_4[i] += p_pos4;
        }

        std::vector<ThickLine> footprint;

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

        footprint.clear();
        for(int i = 1; i < (int)fp_2.size(); i++) {
            ThickLine tl(fp_2[i - 1], fp_2[i]);
            tl.setThickness(2.f);
            // tl.setColor(Config::color_dark_green);
            tl.setColor(sf::Color::Red);
            // tl.setColor(sf::Color::White);
            footprint.push_back(tl);
        }

        for(auto& f_line : footprint) {
            window.draw(f_line);
        }

        footprint.clear();

        for(int i = 1; i < (int)fp_3.size(); i++) {
            ThickLine tl(fp_3[i - 1], fp_3[i]);
            tl.setThickness(2.f);
            // tl.setColor(Config::color_dark_green);
            tl.setColor(sf::Color::Green);
            // tl.setColor(sf::Color::White);
            footprint.push_back(tl);
        }

        for(auto& f_line : footprint) {
            window.draw(f_line);
        }

        footprint.clear();

        for(int i = 1; i < (int)fp_4.size(); i++) {
            ThickLine tl(fp_4[i - 1], fp_4[i]);
            tl.setThickness(2.f);
            // tl.setColor(Config::color_dark_green);
            tl.setColor(sf::Color::Blue);
            // tl.setColor(sf::Color::White);
            footprint.push_back(tl);
        }

        for(auto& f_line : footprint) {
            window.draw(f_line);
        }

        window.display();
        sf::Image screen = window.capture();
        screen.saveToFile("../test_screenshot.jpg");
    }
}

#endif /* __TEST_BASICINSTRUMENT_HPP__*/
