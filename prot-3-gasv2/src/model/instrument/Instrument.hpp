/***********************************************************************************************//**
 *  Common interface for classes representing instruments.
 *  @class      Instrument
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-10
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef INSTRUMENT_HPP
#define INSTRUMENT_HPP

#include "prot.hpp"
#include "common_enum_types.hpp"
#include "EnvModel.hpp"

class Instrument    /* Interface class. */
{
public:
    virtual void setDimensions(EnvModelInfo emi) = 0;
    virtual void setPosition(sf::Vector2f p) = 0;
    virtual void setPosition(sf::Vector3f p) = 0;
    virtual void setVelocity(sf::Vector2f v) = 0;
    virtual void setVelocity(sf::Vector3f v) = 0;
    virtual void enable(void) = 0;
    virtual void disable(void) = 0;
    virtual bool isEnabled(void) const = 0;

    virtual std::vector<sf::Vector2i> getVisibleCells(bool world_cells = false) const = 0;  /* From current position. */
    virtual std::vector<sf::Vector2f> getFootprint(void) const = 0;
    virtual float getResourceRate(std::string rname) const = 0;
};

#endif /* INSTRUMENT_HPP */
