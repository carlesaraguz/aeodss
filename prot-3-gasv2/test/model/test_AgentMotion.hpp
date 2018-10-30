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

        // EXPECT_NE(init_pos, step_pos);
        // EXPECT_NE(init_proj, step_proj);


        /* We will use a known orbit with respective parameters in order to test
         * functions work properly.
         *
         * The orbit used will be: CUTE-1 (CO-55).
         * All parameters are extracted from:
         *      https://celestrak.com/NORAD/elements/cubesat.txt
         *
         *      sma = 1;  (I assume 1 because in celestrack there is no info.)
         *      inc  = 98.6878 (Deg)
         *      raan = 300.3563 (Deg)
         *      ecc  = 0.0010048
         *      argp = 15.2784 (Deg)
         *      init_man = 344.8695 (Deg)
         *      Mean Motion = 14.22139173794016 (Revs/Day)
         *      ang_speed = 0.05925 deg/s
         **/

        // Agent *aptr = new Agent("test_Agent");
        // orbitalParams pars;
        // pars.m_sma  = 1;
        // pars.m_ecc  = 0.0010048;
        // pars.m_raan = 300.3563;
        // pars.m_inc  = 98.6878;
        // pars.m_argp = 15.2784;
        // pars.m_init_mean_an = 344.8695;
        // pars.m_ang_speed = 0.05925;
        //
        // AgentMotion *am = new AgentMotion(aptr, pars);
        //
        // //ASSERT_EQ(am.getAgent(), aptr);
        // ASSERT_EQ(am->getOrbParSma(), pars.m_sma);
        // ASSERT_EQ(am->getOrbParEcc(),(float) pars.m_ecc);
        // ASSERT_EQ(am->getOrbParRaAN(),(float) pars.m_raan);
        // ASSERT_EQ(am->getOrbParInc(),(float) pars.m_inc);
        // ASSERT_EQ(am->getOrbParArgP(),(float) pars.m_argp);
        // ASSERT_EQ(am->getOrbParInitMeanAn(),(float) pars.m_init_mean_an);
        // ASSERT_EQ(am->getOrbParAngSp(),(float) pars.m_ang_speed);
        //
        // /*Here, we will be sure that AgentMotion is well initialized. */
        // sf::Vector3f pos = am->getPosition();
        // EXPECT_NE(pos, sf::Vector3f(-1.f, -1.f, 0));
        // am->propagate(100);
        // EXPECT_NE(am->getPosition(), pos);
        // EXPECT_NE(am->getMeanAnomaly(), pars.m_init_mean_an);

    }
}

#endif /* __TEST_AGENTMOTION_HPP__*/