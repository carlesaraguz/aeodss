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
    virtual ~Resource(void) = default;

    virtual double getCapacity(void) const = 0;
    virtual double getMaxCapacity(void) const = 0;
    virtual double getReservedCapacity(void) const = 0;
    virtual void setMaxCapacity(double c) = 0;
    virtual void setReservedCapacity(double c) = 0;
    virtual void applyOnce(double c) = 0;
    virtual bool applyFor(double c, double t, bool verbose = false) = 0;
    virtual bool isFull(void) const = 0;
    virtual bool isEmpty(void) const = 0;
    virtual bool tryApplyOnce(double c) const = 0;
    virtual void addRate(double dc, Activity* ptr) = 0;
    virtual void removeRate(Activity* ptr) = 0;
    virtual void setName(std::string name) = 0;
    virtual std::string getName(void) const = 0;
    virtual Resource* clone(void) const = 0;
    virtual void showStatus(void) const = 0;
};

#endif /* RESOURCE_HPP */
