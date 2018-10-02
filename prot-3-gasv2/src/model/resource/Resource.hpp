/***********************************************************************************************//**
 *  Generic interface for resources of various types.
 *  @class      Resource
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-07
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "prot.hpp"
#include "TimeStep.hpp"

class Activity;

class Resource : public TimeStep
{
public:
    virtual float getCapacity(void) const = 0;
    virtual float getMaxCapacity(void) const = 0;
    virtual void setMaxCapacity(float c) = 0;
    virtual void applyOnce(float c) = 0;
    virtual void addRate(float dc, Activity* ptr) = 0;
    virtual void removeRate(Activity* ptr) = 0;
    virtual void setName(std::string name) = 0;
    virtual std::string getName(void) const = 0;
};

#endif /* RESOURCE_HPP */
