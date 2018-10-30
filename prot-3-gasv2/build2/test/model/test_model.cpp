/***********************************************************************************************//**
 *  CMake placeholder to implement unit tests with GoogleTest.
 *  @file       test.cpp.in
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-oct-02
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

/***************************************************************************************************
 * (!) This file has been automatically generated by CMake. DO NOT MODIFY!
 **************************************************************************************************/


#include "test_model.hpp"


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
