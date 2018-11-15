/***********************************************************************************************//**
 *  Conical, nadir-pointing instrument.
 *  @class      BasicInstrument
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-10
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef BASIC_INSTRUMENT_HPP
#define BASIC_INSTRUMENT_HPP

#include "prot.hpp"
#include "common_enum_types.hpp"
#include "EnvModel.hpp"
#include "Instrument.hpp"
#include "Random.hpp"
#include "MathUtils.hpp"

/*  NOTE: For this preliminary version, this instrument will be assumed to be a 2-dimensional
 *  capturing device where its swath corresponds to the diameter of the circle that defines its
 *  footprint. Later iterations should consider the following aspects:
 *      - The instrument should be defined with an aperture in degrees, not with a swath.
 *      - Depending on the agent motion type, the instrument should calculate its visibility as the
 *        cone intersection with a sphere.
 **/

class BasicInstrument : public Instrument
{
public:
    BasicInstrument(void);
    BasicInstrument(float aperture);

    /* Instrument interface: */
    void enable(void);
    void disable(void);
    bool isEnabled(void) const { return m_enabled; }
    void setDimensions(EnvModelInfo emi) override { m_env_info = emi; }
    void setPosition(sf::Vector3f p) override;

    /*******************************************************************************************//**
     *  TODO
     **********************************************************************************************/
    std::vector<sf::Vector2i> getVisibleCells(const std::vector<std::vector<sf::Vector3f> >& lut,
        float dist, sf::Vector3f position, bool world_cells, double t = -1.0) const;

    /*******************************************************************************************//**
     *  TODO
     **********************************************************************************************/
    std::vector<sf::Vector2i> getVisibleCells(const std::vector<std::vector<sf::Vector3f> >& lut,
        float dist, sf::Vector2f position, bool world_cells) const;

    /*******************************************************************************************//**
     *  TODO
     **********************************************************************************************/
    std::vector<sf::Vector2i> getVisibleCells(const std::vector<std::vector<sf::Vector3f> >& lut,
        bool world_cells = false) const override;

    std::vector<sf::Vector2f> getFootprint(void) const override;
    float getResourceRate(std::string rname) const override;
    std::map<std::string, float> getResourceRates(void) const;

    /*******************************************************************************************//**
     *  TODO
     **********************************************************************************************/
    float getSwath(sf::Vector3f p, float aperture) const override;

    /*******************************************************************************************//**
     *  TODO
     **********************************************************************************************/
    float getSwath(void) const override { return m_swath; }

    /*******************************************************************************************//**
     *  TODO
     **********************************************************************************************/
    float getAperture(void) const override { return m_aperture; }

    /*******************************************************************************************//**
     *  Compute the instrument slant range at a given observation angle `deg` and position.
     *  @param  deg         The observation angle in degrees. That is: the angle off nadir (i.e.
     *                      slant range for nadir: `deg = 0`). For nadir-pointing instruments, this
     *                      value usually corresponds to a half of the instrument's aperture.
     *  @param  p           The position of the instrument.
     *  @return The distance from the instrument to the surface for the given angle.
     **********************************************************************************************/
    float getSlantRangeAt(float deg, sf::Vector3f p) const override;

private:
    EnvModelInfo m_env_info;
    float m_swath;
    float m_aperture;
    float m_energy_rate;
    float m_storage_rate;
    sf::Vector3f m_position;
    bool m_enabled;

    /*******************************************************************************************//**
     *  Iterates the environment cells (model or world) in 2-dimensions and invokes f() for cells
     *  that are at a distance `r` (or less) from the origin (ox, oy). This function implements a
     *  spiral iteration mechanism: starting at (ox, oy), the iteration moves around the 2D space.
     *  Iteration stops if, after a complete round (i.e. after stepping over each vertex), the
     *  iterator finds all points to be further from the origin.
     *  Iteration boundaries (i.e. max. number of steps) is computed at the beginning. If the
     *  iterator can't complete before reaching the maximum number of steps, the function returns
     *  false and stops looking for points.
     *  If world_cells is set to true, then distances and spiral steps are computed for the actual
     *  cells of the world. Otherwise, the spiral iterates over model cells and computes distance
     *  from model cell to model cell.
     *  @param  ox              The origin of the spiral iteration (x axis, either in world pixels
     *                          or model cell units).
     *  @param  oy              The origin of the spiral iteration (y axis, either in world pixels
     *                          or model cell units).
     *  @param  r               The distance to compare two points with in world pixels or model
     *                          cell units.
     *  @param  world_distance  Whether iteration and distances are world pixels (true) or model
     *                          cell units (false).
     *  @param  f               A function to call for every world/model point that is within
     *                          distance `r`.
     *
     *  @return True when the iterator completed the spiral process before exhausting the maximum
     *          number of iterations. False otherwise.
     **********************************************************************************************/
    bool applyToDistance2D(unsigned int ox, unsigned int oy, float r, bool world_distance = false,
        std::function<void(unsigned int, unsigned int)> f = [](unsigned int, unsigned int) { }) const;

    /*******************************************************************************************//**
     *  Iterates the environment cells (model or world) in 2-dimensions and invokes f() for cells
     *  that are at distance `r` (or less) from the 3-d point p.
     *  @param  ox          The origin of the spiral iteration (x axis, either in world pixels or
     *                      model cell units).
     *  @param  oy          The origin of the spiral iteration (y axis, either in world pixels or
     *                      model cell units).
     *  @param  p           The 3D point to compute distance from (in meters) in the ECI frame.
     *  @param  t           The time at which position `p` is valid.
     *  @param  r           The distance (in meters).
     *  @param  world_cells Whether to iterate over world pixels or model cell units.
     *  @param  f           A function to call for every world pixel or model cell that is within
     *                      distance `r`.
     *  @param  lut         A pre-computed look-up table of positions in ECEF for every cell
     *                      (world or model) that will be used to compute distances.
     **********************************************************************************************/
    void applyToDistance3D(unsigned int ox, unsigned int oy, sf::Vector3f p, double t, float r, bool world_cells = false,
        std::function<void(unsigned int, unsigned int)> f = [](unsigned int, unsigned int) { },
        const std::vector<std::vector<sf::Vector3f> >& lut = { }) const;
};

#include "AgentMotion.hpp"


#endif /* INSTRUMENT_HPP */
