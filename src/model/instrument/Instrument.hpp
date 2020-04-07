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
    virtual void setPosition(sf::Vector3f p) = 0;
    virtual float getAperture(void) const = 0;
    virtual float getSwath(void) const = 0;
    virtual void enable(void) = 0;
    virtual void disable(void) = 0;
    virtual bool isEnabled(void) const = 0;

    virtual float getSlantRangeAt(long double deg, sf::Vector3f p) const = 0;
    virtual float getSwath(sf::Vector3f p, float aperture) const = 0;

    virtual std::vector<sf::Vector2i> getVisibleCells(const std::vector<std::vector<sf::Vector3f> >& lut,
        double dist, sf::Vector3f position, bool world_cells, double t = -1.0) const = 0;
    virtual std::vector<sf::Vector2i> getVisibleCells(double dist, sf::Vector2f position, bool world_cells) const = 0;
    virtual std::vector<sf::Vector2i> getVisibleCells(const std::vector<std::vector<sf::Vector3f> >& lut,
        bool world_cells = false) const = 0;
    virtual std::vector<sf::Vector2i> getVisibleCellsFromTo(const std::vector<std::vector<sf::Vector3f> >& lut,
        double ap, sf::Vector3f p0, sf::Vector3f p1, double t0, double t1, bool world_cells = false) const = 0;

    virtual std::vector<sf::Vector2f> getFootprint(void) const = 0;
    virtual double getResourceRate(std::string rname) const = 0;
    virtual std::map<std::string, double> getResourceRates(void) const = 0;
};

#endif /* INSTRUMENT_HPP */
