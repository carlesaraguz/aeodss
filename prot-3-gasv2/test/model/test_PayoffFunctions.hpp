/***********************************************************************************************//**
 *  Unit-test for PayoffFunctions class.
 *  @class      PayoffFunctionsTest
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-oct-02
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef TEST_PAYOFF_FUNCTIONS_HPP
#define TEST_PAYOFF_FUNCTIONS_HPP

#include "prot.hpp"
#include "PayoffFunctions.hpp"
#include "Activity.hpp"

namespace
{
    class PayoffFunctionsTest : public ::testing::Test
    {
    protected:
        virtual void SetUp(void) {
            PayoffFunctions::bindPayoffFunctions();
        }
    };

    TEST_F(PayoffFunctionsTest, simpleTest)
    {
        auto actptr_a0 = std::make_shared<Activity>("AX", 0);
        auto actptr_a1 = std::make_shared<Activity>("AX", 1);
        auto actptr_b0 = std::make_shared<Activity>("AX", 2);
        auto actptr_b1 = std::make_shared<Activity>("AX", 3);
        auto actptr_test = std::make_shared<Activity>("AY", 4);

        /* Transform into facts: */
        actptr_a0->setConfirmed(true);
        actptr_a1->setConfirmed(true);
        actptr_b0->setConfirmed(true);
        actptr_b1->setConfirmed(true);

        std::vector<std::shared_ptr<Activity> > vec_act_ptr;
        vec_act_ptr.push_back(actptr_a0);
        vec_act_ptr.push_back(actptr_a1);
        vec_act_ptr.push_back(actptr_b0);
        vec_act_ptr.push_back(actptr_b1);

        float rt = -((Config::max_revisit_time - Config::target_revisit_time) / 2.f + Config::target_revisit_time);
        std::vector<std::pair<float, float> > vec_times;
        vec_times.push_back(std::make_pair(rt - 12.f, rt - 11.f));  /* a0 */
        vec_times.push_back(std::make_pair(rt - 10.f, rt));         /* a1 */
        vec_times.push_back(std::make_pair(11.f, 12.f));            /* b0 */
        vec_times.push_back(std::make_pair(13.f, 14.f));            /* b1 */

        float payoff;
        EXPECT_NO_THROW(payoff = PayoffFunctions::f_revisit_time_backwards.first(actptr_test, std::make_pair(0.f, 10.f), { }, { }));
        EXPECT_EQ(payoff, 1.f);
        EXPECT_NO_THROW(payoff = PayoffFunctions::f_revisit_time_backwards.first(actptr_test, std::make_pair(0.f, 10.f), vec_times, vec_act_ptr));
        EXPECT_NEAR(payoff, 0.5f * (Config::max_payoff - Config::min_payoff) + Config::min_payoff, 1e-3);
    }

    TEST_F(PayoffFunctionsTest, test2)
    {
        LogStream::setLogLevel(LogStream::Level::DEBUG);
        auto actptr0 = std::make_shared<Activity>("AX", 0);
        auto actptr1 = std::make_shared<Activity>("AX", 1);
        auto actptr2 = std::make_shared<Activity>("AX", 2);
        auto actptr3 = std::make_shared<Activity>("AX", 3);
        auto actptr_test = std::make_shared<Activity>("AY", 4);

        /* Transform into fact: */
        actptr0->setConfirmed(true);

        /* Define confidence levels: */
        actptr1->setConfidence(0.5f);
        actptr2->setConfidence(0.5f);
        actptr3->setConfidence(0.5f);

        std::vector<std::shared_ptr<Activity> > vec_act_ptr;
        vec_act_ptr.push_back(actptr0);
        vec_act_ptr.push_back(actptr1);
        vec_act_ptr.push_back(actptr2);
        vec_act_ptr.push_back(actptr3);

        float rtmax = -Config::max_revisit_time;
        float rtopt = -Config::target_revisit_time;
        float rtmed = (rtopt + (rtmax - rtopt) / 2.f);
        float m = rtopt / 10.f;
        std::vector<std::pair<float, float> > vec_times;
        vec_times.push_back(std::make_pair(rtmax * 2.f, rtmax));    /* 0 */
        vec_times.push_back(std::make_pair(rtmax - m,   rtmed));    /* 1 */
        vec_times.push_back(std::make_pair(rtmax - m,   rtmed));    /* 2 */
        vec_times.push_back(std::make_pair(rtmed - m,   rtmed));    /* 3 */

        float equiv_rt = (Config::max_revisit_time - Config::target_revisit_time) / 8.f + Config::target_revisit_time;
        float delta = (Config::max_payoff - Config::min_payoff) / (Config::max_revisit_time - Config::target_revisit_time);
        float expected_payoff = delta * (equiv_rt - Config::target_revisit_time) + Config::min_payoff;
        float payoff = PayoffFunctions::f_revisit_time_backwards.first(actptr_test, std::make_pair(0.f, 10.f), vec_times, vec_act_ptr);
        EXPECT_NEAR(payoff, expected_payoff, 1e-3);
    }

    TEST_F(PayoffFunctionsTest, test3)
    {
        LogStream::setLogLevel(LogStream::Level::DEBUG);
        auto actptr0 = std::make_shared<Activity>("AX", 0);
        auto actptr1 = std::make_shared<Activity>("AX", 1);
        auto actptr2 = std::make_shared<Activity>("AX", 2);
        auto actptr3 = std::make_shared<Activity>("AX", 3);
        auto actptr_test = std::make_shared<Activity>("AY", 4);

        /* Transform into fact: */
        actptr0->setConfirmed(true);

        /* Define confidence levels: */
        actptr1->setConfidence(0.5f);
        actptr2->setConfidence(0.5f);
        actptr3->setConfidence(0.5f);

        std::vector<std::shared_ptr<Activity> > vec_act_ptr;
        vec_act_ptr.push_back(actptr0);
        vec_act_ptr.push_back(actptr1);
        vec_act_ptr.push_back(actptr2);
        vec_act_ptr.push_back(actptr3);

        float rtmax = -Config::max_revisit_time;
        float rtopt = -Config::target_revisit_time;
        float rtmed = (rtopt + (rtmax - rtopt) * 3.f / 4.f);
        float m = rtopt / 10.f;
        std::vector<std::pair<float, float> > vec_times;
        vec_times.push_back(std::make_pair(rtmax * 2.f, rtmax));    /* 0 */
        vec_times.push_back(std::make_pair(rtmax - m,   rtmed / 1.f));    /* 1 */
        vec_times.push_back(std::make_pair(rtmax - m,   rtmed / 2.f));    /* 2 */
        vec_times.push_back(std::make_pair(rtmed - m,   rtmed / 4.f));    /* 3 */

        float equiv_rt = (Config::max_revisit_time - Config::target_revisit_time) / 8.f + Config::target_revisit_time;
        float delta = (Config::max_payoff - Config::min_payoff) / (Config::max_revisit_time - Config::target_revisit_time);
        float expected_payoff = delta * (equiv_rt - Config::target_revisit_time) + Config::min_payoff;
        float payoff = PayoffFunctions::f_revisit_time_backwards.first(actptr_test, std::make_pair(0.f, 10.f), vec_times, vec_act_ptr);
        EXPECT_NEAR(payoff, expected_payoff, 1e-3);
    }
}

#endif /* TEST_PAYOFF_FUNCTIONS_HPP */
