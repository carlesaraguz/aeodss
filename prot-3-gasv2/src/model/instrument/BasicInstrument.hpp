/***********************************************************************************************//**
 *  Conical, nadir-pointing instrument.
 *  @class      BasicInstrument
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *              Marc Closa (MC), marc.closa.tarres@alu-etsetb.upc.edu
 *  @date       2018-nov-15
 *  @version    0.2
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

/***********************************************************************************************//**
 *  Nadir-looking instrument with circular footprint. This instrument is defined with an aperture
 *  in degrees. Based on this parameter, the model will compute visible cells and swath both for
 *  2D and 3D environments. In 2D (i.e. Config::motion_model != AgentMotionType::ORBITAL), the swath
 *  equals the aperture value and corresponds to the diameter of a cirlce that does not change with
 *  the instrument position. In 3D (i.e. Config::motion_model == AgentMotionType::ORBITAL), the
 *  swath is computed (in meters) from the instrument aperture and orbital altitude, and both its
 *  2D-projected footprint and visible cells are computed considering the intersection of a cone
 *  (i.e. the instrument's antenna aperture) with a sphere (i.e. the Earth).
 **************************************************************************************************/
class BasicInstrument : public Instrument
{
public:
    /*******************************************************************************************//**
     *  Creates a BasicInstrument with a random aperture in the range [Config::agent_aperture_min,
     *  Config::agent_aperture_miax].
     **********************************************************************************************/
    BasicInstrument(void);

    /*******************************************************************************************//**
     *  Creates a BasicInstrument with the given aperture.
     *  @param  aperture    For 2D motion models: the diameter of the circumference that represents
     *                      the instrument's visibile area. For 3D/orbital motion model: this value
     *                      is the aperture of the instrument in degrees.
     **********************************************************************************************/
    BasicInstrument(float aperture);

    /*******************************************************************************************//**
     *  Enable the instrument. This is just a setter for an internal attribute and does not modify
     *  the behaviour of the class.
     **********************************************************************************************/
    void enable(void);

    /*******************************************************************************************//**
     *  Disable the instrument. This is just a setter for an internal attribute and does not modify
     *  the behaviour of the class.
     **********************************************************************************************/
    void disable(void);

    /*******************************************************************************************//**
     *  Getter for the state of the instrument.
     **********************************************************************************************/
    bool isEnabled(void) const { return m_enabled; }

    /*******************************************************************************************//**
     *  Define environment model dimensions. These dimensions are used in the computation of visible
     *  model cells.
     *  @param  emi     The environment model information for the Agent that owns this instrument.
     **********************************************************************************************/
    void setDimensions(EnvModelInfo emi) override { m_env_info = emi; }

    /*******************************************************************************************//**
     *  Update the position of the instrument.
     *  @param  p       For 2D motion models: position in the world (x, y, z=0), with x and y within
     *                  the boundaries defined with [World::getWidth(), World::getHeight()]. For 3D
     *                  orbital motion model, p is the position in space for this instrument in the
     *                  ECI frame.
     **********************************************************************************************/
    void setPosition(sf::Vector3f p) override;

    /*******************************************************************************************//**
     *  Computes visible cells for an instrument located at position. Visibility is computed with
     *  distance `dist`. Cells that are within `dist` are considered "in visibility." This value
     *  should usually be set to the slant range of the instrument (which does not need to match the
     *  actual slant range of the constructed object.)
     *  @param  lut         A look-up table of the ECEF position of every world or model cell.
     *  @param  dist        The maximum distance to consider a cell as visible. In meters (for 3D
     *                      orbital models.)
     *  @param  position    The position of the instrument. In 2D (x, y, z=0).
     *  @param  world_cells Whether the return value should encompass model cell coordinates (if
     *                      false) or world cordinates in pixels (if true).
     *  @param  t           The time at which the position is valid (needed to transform ECI to
     *                      ECEF). If set to -1.0, this function will consider the current time.
     *                      This argument is only needed in 3d/orbital motion models.
     *  @note For a 2D version of this function, see the other overloads of getVisibleCells.
     **********************************************************************************************/
    std::vector<sf::Vector2i> getVisibleCells(const std::vector<std::vector<sf::Vector3f> >& lut,
        float dist, sf::Vector3f position, bool world_cells, double t = -1.0) const;

    /*******************************************************************************************//**
     *  Computes visible cells for an instrument located at the given 2D position. Visibility is
     *  computed with distance `dist`. Cells that are within `dist` are considered "in visibility."
     *  This value should usually correspond (for a 2D motion model) to the radius of the instrument
     *  footprint (i.e. half its swath).
     *  @param  dist        The maximum distance to consider a cell as visible. In either world
     *                      pixels or model cell units.
     *  @param  position    The position of the instrument.
     *  @param  world_cells Whether the return value should encompass model cell coordinates (if
     *                      false) or world cordinates in pixels (if true).
     *  @note This function should only be used in 2D motion models.
     **********************************************************************************************/
    std::vector<sf::Vector2i> getVisibleCells(float dist, sf::Vector2f position, bool world_cells) const;

    /*******************************************************************************************//**
     *  Computes visible cells for the actual instrument (i.e. taking its current position and
     *  aperture). In 2D, this function computes cells that are at a distance of swath/2. In 3D,
     *  this function computes visible cells with the instrument's slant range.
     *  @param  lut         A look-up table of the ECEF position of every world or model cell.
     *  @param  world_cells Whether the return value should encompass model cell coordinates (if
     *                      false) or world cordinates in pixels (if true).
     **********************************************************************************************/
    std::vector<sf::Vector2i> getVisibleCells(const std::vector<std::vector<sf::Vector3f> >& lut,
        bool world_cells = false) const override;

    /*******************************************************************************************//**
     *  TODO
     **********************************************************************************************/
    std::vector<sf::Vector2f> getFootprint(void) const override;

    /*******************************************************************************************//**
     *  Returns the consumption rate for the resource named `rname`.
     *  @param  rname   Name of the resource.
     **********************************************************************************************/
    float getResourceRate(std::string rname) const override;

    /*******************************************************************************************//**
     *  Gets the list of consumption rates for this instrument.
     *  @return     A map of resource rates identified with a string key that corresponds to the
     *              resources name.
     **********************************************************************************************/
    std::map<std::string, float> getResourceRates(void) const;

    /*******************************************************************************************//**
     *  Compute the swath for a nadir-looking instrument located at `p` with the given aperture.
     *  This function is only valid for 3D/orbital motion models.
     *  @param  p           Position of the instrument.
     *  @param  aperture    Aperture of the instrument in degrees.
     **********************************************************************************************/
    float getSwath(sf::Vector3f p, float aperture) const override;

    /*******************************************************************************************//**
     *  Getter for the instrument swath. This value is computed at construction time. For 2D motion
     *  models, this will be equal to the instrument's aperture.
     **********************************************************************************************/
    float getSwath(void) const override { return m_swath; }

    /*******************************************************************************************//**
     *  Getter for the instrument aperture.
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
    EnvModelInfo m_env_info;    /**< Information about the environment model size. */
    float m_swath;              /**< Instrument swath, in pixels (2D) or meters (3D). */
    float m_aperture;           /**< Aperture of the instrument in degrees. */
    float m_energy_rate;        /**< Energy per time unit consumed when the instrument is enabled. */
    float m_storage_rate;       /**< Storage per time unit consumed when the instrument is enabled. */
    sf::Vector3f m_position;    /**< Current position of the instrument. */
    bool m_enabled;             /**< Whether the instrument is enabled (true) or not (false). */

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
