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

        double rt = -((Config::max_revisit_time - Config::target_revisit_time) / 2.0 + Config::target_revisit_time);
        std::vector<std::vector<std::pair<double, double> > > vec_times;
        vec_times.push_back({std::make_pair(rt - 12.0, rt - 11.0)});    /* a0 */
        vec_times.push_back({std::make_pair(rt - 10.0, rt)});           /* a1 */
        vec_times.push_back({std::make_pair(11.0, 12.0)});              /* b0 */
        vec_times.push_back({std::make_pair(13.0, 14.0)});              /* b1 */

        float payoff;
        EXPECT_NO_THROW(payoff = PayoffFunctions::f_revisit_time_backwards.first(std::make_pair(0.0, 10.0), { }, { }));
        EXPECT_EQ(payoff, 1.f);
        EXPECT_NO_THROW(payoff = PayoffFunctions::f_revisit_time_backwards.first(std::make_pair(0.0, 10.0), vec_times, vec_act_ptr));
        EXPECT_NEAR(payoff, 0.5f * (Config::max_payoff - Config::min_payoff) + Config::min_payoff, 1e-3);
    }

    TEST_F(PayoffFunctionsTest, test2)
    {
        LogStream::setLogLevel(LogStream::Level::DEBUG);
        auto actptr0 = std::make_shared<Activity>("AX", 0);
        auto actptr1 = std::make_shared<Activity>("AX", 1);
        auto actptr2 = std::make_shared<Activity>("AX", 2);
        auto actptr3 = std::make_shared<Activity>("AX", 3);

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

        double rtmax = -Config::max_revisit_time;
        double rtopt = -Config::target_revisit_time;
        double rtmed = (rtopt + (rtmax - rtopt) / 2.0);
        double m = rtopt / 10.0;
        std::vector<std::vector<std::pair<double, double> > > vec_times;
        vec_times.push_back({std::make_pair(rtmax * 2.0, rtmax)});    /* 0 */
        vec_times.push_back({std::make_pair(rtmax - m,   rtmed)});    /* 1 */
        vec_times.push_back({std::make_pair(rtmax - m,   rtmed)});    /* 2 */
        vec_times.push_back({std::make_pair(rtmed - m,   rtmed)});    /* 3 */

        double equiv_rt = (Config::max_revisit_time - Config::target_revisit_time) / 8.0 + Config::target_revisit_time;
        double delta = (Config::max_payoff - Config::min_payoff) / (Config::max_revisit_time - Config::target_revisit_time);
        double expected_payoff = delta * (equiv_rt - Config::target_revisit_time) + Config::min_payoff;
        float payoff = PayoffFunctions::f_revisit_time_backwards.first(std::make_pair(0.0, 10.0), vec_times, vec_act_ptr);
        EXPECT_NEAR(payoff, expected_payoff, 1e-3);
    }

    TEST_F(PayoffFunctionsTest, test3)
    {
        LogStream::setLogLevel(LogStream::Level::DEBUG);
        auto actptr0 = std::make_shared<Activity>("AX", 0);
        auto actptr1 = std::make_shared<Activity>("AX", 1);
        auto actptr2 = std::make_shared<Activity>("AX", 2);
        auto actptr3 = std::make_shared<Activity>("AX", 3);

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

        double rtmax = -Config::max_revisit_time;
        double rtopt = -Config::target_revisit_time;
        double rtmed = (rtopt + (rtmax - rtopt) * 3.0 / 4.0);
        double m = rtopt / 10.0;
        std::vector<std::vector<std::pair<double, double> > > vec_times;
        vec_times.push_back({std::make_pair(rtmax * 2.0, rtmax)});          /* 0 */
        vec_times.push_back({std::make_pair(rtmax - m,   rtmed / 1.0)});    /* 1 */
        vec_times.push_back({std::make_pair(rtmax - m,   rtmed / 2.0)});    /* 2 */
        vec_times.push_back({std::make_pair(rtmed - m,   rtmed / 4.0)});    /* 3 */

        double equiv_rt = (Config::max_revisit_time - Config::target_revisit_time) / 8.0 + Config::target_revisit_time;
        double delta = (Config::max_payoff - Config::min_payoff) / (Config::max_revisit_time - Config::target_revisit_time);
        double expected_payoff = delta * (equiv_rt - Config::target_revisit_time) + Config::min_payoff;
        float payoff = PayoffFunctions::f_revisit_time_backwards.first(std::make_pair(0.0, 10.0), vec_times, vec_act_ptr);
        EXPECT_NEAR(payoff, expected_payoff, 1e-3);
    }
}

#endif /* TEST_PAYOFF_FUNCTIONS_HPP */
