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
#include <gtest/gtest.h>

#include "PayoffFunctions.hpp"

namespace
{
    class PayoffFunctionsTest : public ::testing::Test
    {
    protected:
        virtual void SetUp() { }
    };

    TEST_F(PayoffFunctionsTest, simpleTest) {
        EXPECT_TRUE(false);
    }
}

#endif /* TEST_PAYOFF_FUNCTIONS_HPP */
