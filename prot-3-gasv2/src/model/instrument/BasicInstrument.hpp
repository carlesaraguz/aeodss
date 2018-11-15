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
    BasicInstrument(float world_swath);     /* TODO: this is just a temporary version. */

    /* Instrument interface: */
    void enable(void);
    void disable(void);
    bool isEnabled(void) const { return m_enabled; }
    void setDimensions(EnvModelInfo emi) override { m_env_info = emi; }
    void setPosition(sf::Vector3f p) override { m_position = p; }
    std::vector<sf::Vector2i> getVisibleCells(bool world_cells = false) const override;
    std::vector<sf::Vector2i> getVisibleCells(float swath, sf::Vector2f position) const;

    /*******************************************************************************************//**
     *  Computes the footprint of the instrument from the instrument apperture and ECI Coordinates.
     *  Positions given are alredy pojected in the equirectangular map. The computation principles
     *  are taken from the intersection of 2 spheres an from there, using some trigonometry,
     *  we are able to compute the needed points of the footprint.
     *  For more accurate information, you can visit:
     *  https://math.stackexchange.com/questions/73237/parametric-equation-of-a-circle-in-3d-space
     **********************************************************************************************/
    std::vector<sf::Vector2f> getFootprint(void) const override;
    float getResourceRate(std::string rname) const override;
    std::map<std::string, float> getResourceRates(void) const;
    float getSwath(void) const { return m_swath; }
    sf::Vector3f getPosition(void) const { return m_position; }

private:
    EnvModelInfo m_env_info;
    float m_swath;
    float m_energy_rate;
    float m_storage_rate;
    sf::Vector3f m_position;
    bool m_enabled;

    bool applyToDistance(unsigned int ox, unsigned int oy, float r, bool world_distance = false,
        std::function<void(unsigned int, unsigned int)> f_true = [](unsigned int, unsigned int) { },
        std::function<void(unsigned int, unsigned int)> f_false = [](unsigned int, unsigned int) { }) const;
};

#include "AgentMotion.hpp"


#endif /* INSTRUMENT_HPP */
