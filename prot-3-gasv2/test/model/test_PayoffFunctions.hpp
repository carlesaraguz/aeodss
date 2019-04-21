/***********************************************************************************************//**
 *  Unit-test for PayoffFunctions class.
 *  @class      PayoffFunctionsTest
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2019-abr-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef TEST_PAYOFF_FUNCTIONS_HPP
#define TEST_PAYOFF_FUNCTIONS_HPP

#include "prot.hpp"
#include "PayoffFunctions.hpp"
#include "Activity.hpp"

#define EPSILON      9.999e-5

namespace
{
    class PayoffFunctionsTest : public ::testing::Test
    {
    protected:
        EnvCell::EnvCellPayoffFunc f_po_forward;
        EnvCell::EnvCellPayoffFunc f_po_backward;
        std::vector<std::vector<std::pair<double, double> > > bts;
        std::vector<std::shared_ptr<Activity> > bs;

        virtual void SetUp(void) {
            // LogStream::setLogLevel(LogStream::Level::DEBUG);
            PayoffFunctions::bindPayoffFunctions();
            Config::goal_target = 1.0;
            f_po_forward = PayoffFunctions::f_revisit_time_forwards.first;
            f_po_backward = PayoffFunctions::f_revisit_time_backwards.first;
            bts.clear();
            time_intervals.clear();
            bs.clear();
        }
        void helper(std::vector<std::vector<std::pair<double, double> > >& bts,
            std::vector<std::shared_ptr<Activity> >& bs,
            bool confirmed, double ts, double te, float confidence = 0.5f)
        {
            auto act = std::make_shared<Activity>("aid");
            if(confirmed) {
                act->setConfirmed(confirmed);
            } else {
                act->setConfidence(confidence);
            }
            bs.push_back(act);
            std::vector<std::pair<double, double> > t_ints;
            t_ints.push_back(std::make_pair(ts, te));
            bts.push_back(t_ints);
        }
        void clearAll(void)
        {
            bts.clear();
            bs.clear();
        }
    private:
        std::vector<std::pair<double, double> > time_intervals;
    };

    TEST_F(PayoffFunctionsTest, PayoffSigmoid)
    {
        Config::payoff_model = PayoffModel::SIGMOID;
        Config::payoff_steepness = 20.f;
        std::vector<float> po_correct_k20 = {
            0.00000f, 0.00000f, 0.00000f, 0.00000f, 0.00001f, 0.00005f, 0.00034f,
            0.00247f, 0.01799f, 0.11920f, 0.50000f, 0.88080f, 0.98201f, 0.99753f,
            0.99966f, 0.99995f, 0.99999f, 1.00000f, 1.00000f, 1.00000f, 1.00000f
        };
        for(unsigned int i = 0; i < po_correct_k20.size(); i++) {
            double rt = i * 0.1;
            EXPECT_NEAR(po_correct_k20[i], PayoffFunctions::payoff(rt), 0.0000099999);
        }
        Config::payoff_steepness = 10.f;
        std::vector<float> po_correct_k10 = {
            4.5398e-05f, 1.2339e-04f, 3.3535e-04f, 9.1105e-04f, 2.4726e-03f, 6.6929e-03f,
            1.7986e-02f, 4.7426e-02f, 1.1920e-01f, 2.6894e-01f, 5.0000e-01f, 7.3106e-01f,
            8.8080e-01f, 9.5257e-01f, 9.8201e-01f, 9.9331e-01f, 9.9753e-01f, 9.9909e-01f,
            9.9966e-01f, 9.9988e-01f, 9.9995e-01f
        };
        for(unsigned int i = 0; i < po_correct_k10.size(); i++) {
            double rt = i * 0.1;
            EXPECT_NEAR(po_correct_k10[i], PayoffFunctions::payoff(rt), 0.0000099999);
        }
    }

    TEST_F(PayoffFunctionsTest, PayoffLinear)
    {
        Config::payoff_model = PayoffModel::LINEAR;
        Config::goal_min = 0.0;
        Config::goal_max = 2.0;

        Config::payoff_mid = 0.5f;
        std::vector<float> po_correct_mid05 = {
            0.00000f, 0.02500f, 0.05000f, 0.07500f, 0.10000f, 0.12500f, 0.15000f, 0.17500f, 0.20000f,
            0.22500f, 0.25000f, 0.27500f, 0.30000f, 0.32500f, 0.35000f, 0.37500f, 0.40000f, 0.42500f,
            0.45000f, 0.47500f, 0.50000f, 0.52500f, 0.55000f, 0.57500f, 0.60000f, 0.62500f, 0.65000f,
            0.67500f, 0.70000f, 0.72500f, 0.75000f, 0.77500f, 0.80000f, 0.82500f, 0.85000f, 0.87500f,
            0.90000f, 0.92500f, 0.95000f, 0.97500f, 1.00000f
        };
        for(unsigned int i = 0; i < po_correct_mid05.size(); i++) {
            double rt = 0.05 * i;
            EXPECT_NEAR(po_correct_mid05[i], PayoffFunctions::payoff(rt), 0.00000999999);
        }
        Config::payoff_mid = 0.8f;
        std::vector<float> po_correct_mid08 = {
            0.00000f, 0.04000f, 0.08000f, 0.12000f, 0.16000f, 0.20000f, 0.24000f, 0.28000f, 0.32000f,
            0.36000f, 0.40000f, 0.44000f, 0.48000f, 0.52000f, 0.56000f, 0.60000f, 0.64000f, 0.68000f,
            0.72000f, 0.76000f, 0.80000f, 0.81000f, 0.82000f, 0.83000f, 0.84000f, 0.85000f, 0.86000f,
            0.87000f, 0.88000f, 0.89000f, 0.90000f, 0.91000f, 0.92000f, 0.93000f, 0.94000f, 0.95000f,
            0.96000f, 0.97000f, 0.98000f, 0.99000f, 1.00000f
        };
        for(unsigned int i = 0; i < po_correct_mid08.size(); i++) {
            double rt = 0.05 * i;
            EXPECT_NEAR(po_correct_mid08[i], PayoffFunctions::payoff(rt), 0.00000999999);
        }
        Config::payoff_mid = 0.2f;
        std::vector<float> po_correct_mid02 = {
            0.00000f, 0.01000f, 0.02000f, 0.03000f, 0.04000f, 0.05000f, 0.06000f, 0.07000f, 0.08000f,
            0.09000f, 0.10000f, 0.11000f, 0.12000f, 0.13000f, 0.14000f, 0.15000f, 0.16000f, 0.17000f,
            0.18000f, 0.19000f, 0.20000f, 0.24000f, 0.28000f, 0.32000f, 0.36000f, 0.40000f, 0.44000f,
            0.48000f, 0.52000f, 0.56000f, 0.60000f, 0.64000f, 0.68000f, 0.72000f, 0.76000f, 0.80000f,
            0.84000f, 0.88000f, 0.92000f, 0.96000f, 1.00000f
        };
        for(unsigned int i = 0; i < po_correct_mid02.size(); i++) {
            double rt = 0.05 * i;
            EXPECT_NEAR(po_correct_mid02[i], PayoffFunctions::payoff(rt), 0.00000999999);
        }
        EXPECT_EQ(PayoffFunctions::payoff(-1.0), 0.0);
        EXPECT_EQ(PayoffFunctions::payoff( 5.0), 1.0);
    }

    TEST_F(PayoffFunctionsTest, BackwardsFacts)
    {
        Config::payoff_model = PayoffModel::LINEAR;
        Config::payoff_mid = 0.5f;
        Config::goal_min = 0.0;
        Config::goal_max = 2.0;

        /* Basic payoff: one single fact to check with (I): */
        helper(bts, bs, true, -2.5, -2.0);
        EXPECT_NEAR(f_po_backward(std::make_pair(1.0, 1.5), bts, bs), 1.0, EPSILON);
        helper(bts, bs, true, -1.0, 0.0);
        EXPECT_NEAR(f_po_backward(std::make_pair(1.0, 1.5), bts, bs), 0.5, EPSILON);
        helper(bts, bs, true, -1.0, 0.5);
        EXPECT_NEAR(f_po_backward(std::make_pair(1.0, 1.5), bts, bs), 0.25, EPSILON);
        helper(bts, bs, true, -1.0, 1.0);
        EXPECT_NEAR(f_po_backward(std::make_pair(1.0, 1.5), bts, bs), 0.0, EPSILON);
        clearAll();

        /* Basic payoff: one single fact to check with (II): */
        helper(bts, bs, true, 0.0, 0.5);
        helper(bts, bs, true, 1.5, 1.6);    /* Starts later than 1.0. */
        EXPECT_NEAR(f_po_backward(std::make_pair(1.0, 1.5), bts, bs), 0.25, EPSILON);
        helper(bts, bs, true, 0.9, 1.2);    /* Overlaps. */
        EXPECT_NEAR(f_po_backward(std::make_pair(1.0, 1.5), bts, bs), 0.0, EPSILON);
        clearAll();
    }

    TEST_F(PayoffFunctionsTest, BackwardsMixed)
    {
        Config::payoff_model = PayoffModel::LINEAR;
        Config::payoff_mid = 0.5f;
        Config::goal_min = 0.0;      /* PO = 0.0 */
        Config::goal_target = 10.0;  /* PO = 0.5 */
        Config::goal_max = 20.0;     /* PO = 1.0 */

        /* Mixed payoff: one fact and one undecided. */
        helper(bts, bs, true,  0.0,  5.0);          /* RT(worst) = 10.0 --> PO(best) = 0.5. */
        helper(bts, bs, false, 0.0,  2.5, 0.5f);    /* Ignored. */
        helper(bts, bs, false, 6.0, 10.0, 0.5f);    /* RT = 5.0 --> PO = 0.5-0.25/2 = 0.375 */
        EXPECT_NEAR(f_po_backward(std::make_pair(15.0, 16.0), bts, bs), 0.5-0.25/2, EPSILON);
        helper(bts, bs, false, 6.0, 10.0, 0.5f);    /* RT = 5.0 --> PO = 0.375-(0.375-0.25)/2 = 0.3125 */
        EXPECT_NEAR(f_po_backward(std::make_pair(15.0, 16.0), bts, bs), 0.3125, EPSILON);

        /* Multiple undecided, no facts: */
        Config::goal_min = 0.0;     /* PO = 0.0 */
        Config::goal_target = 2.5;  /* PO = 0.5 */
        Config::goal_max = 5.0;     /* PO = 1.0 */
        clearAll();
        helper(bts, bs, false, -1.0, 0.0, 0.5f);    /* RT(worst) = 5.0 --> PO(best) = 1.0. */
        helper(bts, bs, false, 0.0, 0.5, 0.5f);     /* RT = 4.5 --> PO = 1.0-(1.0-4.5/5)*0.5 = 0.95   */
        helper(bts, bs, false, 0.0, 1.0, 0.5f);     /* RT = 4.0 --> PO = 0.95-(0.95-4.0/5)*0.5 = 0.875  */
        helper(bts, bs, false, 0.0, 1.5, 0.5f);     /* RT = 3.5 --> PO = 0.875-(0.875-3.5/5)*0.5 = 0.7875 */
        helper(bts, bs, false, 0.0, 2.0, 0.5f);     /* RT = 3.0 --> PO = 0.7875-(0.7875-3.0/5)*0.5 = 0.69375*/
        helper(bts, bs, false, 0.0, 2.5, 0.5f);     /* RT = 2.5 --> PO = 0.69375-(0.69375-2.5/5)*0.5 = 0.596875 */
        helper(bts, bs, false, 0.0, 3.0, 0.5f);     /* RT = 2.0 --> PO = 0.596875-(0.596875-2.0/5)*0.5 = 0.4984375 */
        helper(bts, bs, false, 0.0, 3.5, 0.5f);     /* RT = 1.5 --> PO = 0.4984375-(0.4984375-1.5/5)*0.5 = 0.39921875 */
        helper(bts, bs, false, 0.0, 4.0, 0.5f);     /* RT = 1.0 --> PO = 0.39921875-(0.39921875-1.0/5)*0.5 = 0.299609375 */
        helper(bts, bs, false, 0.0, 4.5, 0.5f);     /* RT = 0.5 --> PO = 0.299609375-(0.299609375-0.5/5)*0.5 = 0.1998046875 */
        helper(bts, bs, false, 0.0, 5.0, 0.5f);     /* RT = 0.0 --> PO = 0.1998046875-(0.1998046875-0.0/5)*0.5 = 0.09990234375 */
        EXPECT_NEAR(f_po_backward(std::make_pair(5.0, 6.0), bts, bs), 0.09990234375, EPSILON);
    }

    TEST_F(PayoffFunctionsTest, BackwardsOverlap)
    {
        Config::payoff_model = PayoffModel::LINEAR;
        Config::payoff_mid = 0.5f;
        Config::goal_min = 0.0;     /* PO = 0.0 */
        Config::goal_target = 10.0; /* PO = 0.5 */
        Config::goal_max = 20.0;    /* PO = 1.0 */

        helper(bts, bs, true, 0.0, 10.0);  /* Fact overlaps activity completely: */
        EXPECT_NEAR(f_po_backward(std::make_pair(5.0, 9.0), bts, bs), 0.0, EPSILON)
            << "Fact overlaps activity completely.";
        clearAll();

        helper(bts, bs, true, 0.0, 10.0);  /* Fact overlaps activity partially: */
        EXPECT_NEAR(f_po_backward(std::make_pair(5.0, 15.0), bts, bs), 0.0, EPSILON)
            << "Fact overlaps activity partially.";
        clearAll();

        helper(bts, bs, true, 0.0, 10.0);  /* Fact overlaps activity at start time: */
        EXPECT_NEAR(f_po_backward(std::make_pair(0.0, 5.0), bts, bs), 0.0, EPSILON)
            << "Fact overlaps activity at start time.";
        clearAll();

        helper(bts, bs, false, 0.0, 10.0, 0.5f);  /* A single undecided overlaps activity completely: */
        EXPECT_NEAR(f_po_backward(std::make_pair(5.0, 9.0), bts, bs), 0.5, EPSILON)
            << "A single undecided overlaps activity completely.";
        clearAll();

        helper(bts, bs, false, 0.0, 10.0, 0.5f);  /* A single undecided overlaps activity partially: */
        EXPECT_NEAR(f_po_backward(std::make_pair(5.0, 15.0), bts, bs), 0.5, EPSILON)
            << "A single undecided overlaps activity partially.";
        clearAll();

        /* Two undecided; one overlaps activity partially: */
        helper(bts, bs, false,  0.0,  7.5, 0.5f);   /* RT(worst) = 17.5 --> PO(best) = 0.875 */
        helper(bts, bs, false, 20.0, 26.0, 0.5f);   /* Overlaps partially --> TEnd = 25 --> RT = 0 --> 0.875-(0.875)*0.5 */
        EXPECT_NEAR(f_po_backward(std::make_pair(25.0, 27.0), bts, bs), 0.4375, EPSILON)
            << "Two undecided; one overlaps activity partially.";
        clearAll();
    }

    TEST_F(PayoffFunctionsTest, ForwardsFacts)
    {
        Config::payoff_model = PayoffModel::LINEAR;
        Config::payoff_mid = 0.5f;
        Config::goal_min = 0.0;
        Config::goal_max = 2.0;

        /* Basic payoff: one single fact to check with (I): */
        helper(bts, bs, true, 12.0, 12.5);
        EXPECT_NEAR(f_po_forward(std::make_pair(0.0, 10.0), bts, bs), 1.0, EPSILON);
        helper(bts, bs, true, 11.0, 11.5);
        EXPECT_NEAR(f_po_forward(std::make_pair(0.0, 10.0), bts, bs), 0.5, EPSILON);
        helper(bts, bs, true, 10.5, 11.0);
        EXPECT_NEAR(f_po_forward(std::make_pair(0.0, 10.0), bts, bs), 0.25, EPSILON);
        helper(bts, bs, true, 10.0, 10.5);
        EXPECT_NEAR(f_po_forward(std::make_pair(0.0, 10.0), bts, bs), 0.0, EPSILON);
        clearAll();

        /* Basic payoff: one single fact to check with (II): */
        helper(bts, bs, true, 2.5, 3.5);    /* RT=0.5 --> PO=0.25*/
        helper(bts, bs, true, 0.5, 1.5);    /* Ends earlier than 2.0. */
        EXPECT_NEAR(f_po_forward(std::make_pair(1.0, 2.0), bts, bs), 0.25, EPSILON);
        helper(bts, bs, true, 1.9, 2.2);    /* Overlaps. */
        EXPECT_NEAR(f_po_forward(std::make_pair(1.0, 2.0), bts, bs), 0.0, EPSILON);
        clearAll();
    }

    TEST_F(PayoffFunctionsTest, ForwardsMixed)
    {
        Config::payoff_model = PayoffModel::LINEAR;
        Config::payoff_mid = 0.5f;
        Config::goal_min = 0.0;      /* PO = 0.0 */
        Config::goal_target = 10.0;  /* PO = 0.5 */
        Config::goal_max = 20.0;     /* PO = 1.0 */

        /* Mixed payoff: one fact and one undecided. */
        helper(bts, bs, true,  15.0, 20.0);         /* RT(worst) = 10.0 --> PO(best) = 0.5. */
        helper(bts, bs, false, 16.0, 21.0, 0.5f);   /* Ignored. */
        helper(bts, bs, false, 10.0, 11.0, 0.5f);   /* RT = 5.0 --> PO = 0.5-0.25/2 = 0.375 */
        EXPECT_NEAR(f_po_forward(std::make_pair(0.0, 5.0), bts, bs), 0.5-0.25/2, EPSILON);
        helper(bts, bs, false, 10.0, 11.0, 0.5f);   /* RT = 5.0 --> PO = 0.375-(0.375-0.25)/2 = 0.3125 */
        EXPECT_NEAR(f_po_forward(std::make_pair(0.0, 5.0), bts, bs), 0.3125, EPSILON);

        /* Multiple undecided, no facts: */
        Config::goal_min = 0.0;     /* PO = 0.0 */
        Config::goal_target = 2.5;  /* PO = 0.5 */
        Config::goal_max = 5.0;     /* PO = 1.0 */
        clearAll();
        helper(bts, bs, false, 10.0, 10.5, 0.5f);   /* RT(worst) = 5.0 --> PO(best) = 1.0. */
        helper(bts, bs, false,  9.5, 10.0, 0.5f);   /* RT = 4.5 --> PO = 1.0-(1.0-4.5/5)*0.5 = 0.95   */
        helper(bts, bs, false,  9.0,  9.5, 0.5f);   /* RT = 4.0 --> PO = 0.95-(0.95-4.0/5)*0.5 = 0.875  */
        helper(bts, bs, false,  8.5,  9.0, 0.5f);   /* RT = 3.5 --> PO = 0.875-(0.875-3.5/5)*0.5 = 0.7875 */
        helper(bts, bs, false,  8.0,  8.5, 0.5f);   /* RT = 3.0 --> PO = 0.7875-(0.7875-3.0/5)*0.5 = 0.69375*/
        helper(bts, bs, false,  7.5,  8.0, 0.5f);   /* RT = 2.5 --> PO = 0.69375-(0.69375-2.5/5)*0.5 = 0.596875 */
        helper(bts, bs, false,  7.0,  7.5, 0.5f);   /* RT = 2.0 --> PO = 0.596875-(0.596875-2.0/5)*0.5 = 0.4984375 */
        helper(bts, bs, false,  6.5,  7.0, 0.5f);   /* RT = 1.5 --> PO = 0.4984375-(0.4984375-1.5/5)*0.5 = 0.39921875 */
        helper(bts, bs, false,  6.0,  6.5, 0.5f);   /* RT = 1.0 --> PO = 0.39921875-(0.39921875-1.0/5)*0.5 = 0.299609375 */
        helper(bts, bs, false,  5.5,  6.0, 0.5f);   /* RT = 0.5 --> PO = 0.299609375-(0.299609375-0.5/5)*0.5 = 0.1998046875 */
        helper(bts, bs, false,  5.0,  5.5, 0.5f);   /* RT = 0.0 --> PO = 0.1998046875-(0.1998046875-0.0/5)*0.5 = 0.09990234375 */
        EXPECT_NEAR(f_po_forward(std::make_pair(0.0, 5.0), bts, bs), 0.09990234375, EPSILON);
    }

    TEST_F(PayoffFunctionsTest, ForwardsOverlap)
    {
        Config::payoff_model = PayoffModel::LINEAR;
        Config::payoff_mid = 0.5f;
        Config::goal_min = 0.0;     /* PO = 0.0 */
        Config::goal_target = 10.0; /* PO = 0.5 */
        Config::goal_max = 20.0;    /* PO = 1.0 */

        helper(bts, bs, true, 0.0, 10.0);  /* Fact overlaps activity completely: */
        EXPECT_NEAR(f_po_forward(std::make_pair(5.0, 9.0), bts, bs), 0.0, EPSILON)
            << "Fact overlaps activity completely.";
        clearAll();

        helper(bts, bs, true, 5.0, 15.0);  /* Fact overlaps activity partially: */
        EXPECT_NEAR(f_po_forward(std::make_pair(0.0, 10.0), bts, bs), 0.0, EPSILON)
            << "Fact overlaps activity partially.";
        clearAll();

        helper(bts, bs, true, 5.0, 10.0);  /* Fact overlaps activity at end time: */
        EXPECT_NEAR(f_po_forward(std::make_pair(0.0, 10.0), bts, bs), 0.0, EPSILON)
            << "Fact overlaps activity at end time.";
        clearAll();

        helper(bts, bs, false, 0.0, 10.0, 0.5f);  /* A single undecided overlaps activity completely: */
        EXPECT_NEAR(f_po_forward(std::make_pair(5.0, 9.0), bts, bs), 0.5, EPSILON)
            << "A single undecided overlaps activity completely.";
        clearAll();

        helper(bts, bs, false, 5.0, 15.0, 0.5f);  /* A single undecided overlaps activity partially: */
        EXPECT_NEAR(f_po_forward(std::make_pair(0.0, 10.0), bts, bs), 0.5, EPSILON)
            << "A single undecided overlaps activity partially.";
        clearAll();

        /* Two undecided; one overlaps activity partially: */
        helper(bts, bs, false, 27.5, 28.0, 0.5f);   /* RT(worst) = 17.5 --> PO(best) = 0.875 */
        helper(bts, bs, false,  7.5, 15.0, 0.5f);   /* Overlaps partially --> TStart = 5 --> RT = 0 --> 0.875-(0.875)*0.5 */
        EXPECT_NEAR(f_po_forward(std::make_pair(5.0, 10.0), bts, bs), 0.4375, EPSILON)
            << "Two undecided; one overlaps activity partially.";
        clearAll();
    }
}

#endif /* TEST_PAYOFF_FUNCTIONS_HPP */
