#ifndef __TEST_AGENTMOTION_HPP__
#define __TEST_AGENTMOTION_HPP__

#include <gtest/gtest.h>

#include "AgentMotion.hpp"
#include "prot.hpp"

CREATE_LOGGER(test_AgentMotion)


namespace
{
    TEST(AgentMotionTest, testAgentMotion)
    {
        AgentMotion *am = new AgentMotion(nullptr, 0.f);

        std::cout << am->getSMA() << '\n';

        std::cout << "v = " << '\n';
        for (unsigned int i = 0; i < 300; i++) {
            am->step();

            sf::Vector3f step_pos  = am->getPosition();
            // sf::Vector2f step_proj = am->getProjection2D();

            std::cout << step_pos.x <<"," << step_pos.y <<"," << step_pos.z << "\n";
            // std::cout << step_proj.x <<", " << step_proj.y  << '\n';
        }

        std::cout << '\n' << '\n'<< "**************************" << '\n' << '\n';

        for (unsigned int i = 0; i < 300; i++) {
            am->step();

            sf::Vector2f step_pos  = am->getProjection2D();
            // sf::Vector2f step_proj = am->getProjection2D();

            std::cout << step_pos.x <<"," << step_pos.y << "\n";
            // std::cout << step_proj.x <<", " << step_proj.y  << '\n';
        }
    }
}

#endif /* __TEST_AGENTMOTION_HPP__*/
