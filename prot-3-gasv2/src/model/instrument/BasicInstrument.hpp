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
    void setPosition(sf::Vector2f p) override { setPosition(sf::Vector3f(p.x, p.y, 0.f)); }
    void setPosition(sf::Vector3f p) override { m_position = p; }
    void setVelocity(sf::Vector2f v) override { setVelocity(sf::Vector3f(v.x, v.y, 0.f)); }
    void setVelocity(sf::Vector3f) override { /* Does nothing. TODO Preliminary version. */ }
    std::vector<sf::Vector2i> getVisibleCells(bool world_cells = false) const override;
    std::vector<sf::Vector2i> getVisibleCells(float swath, sf::Vector2f position) const;
    std::vector<sf::Vector2f> getFootprint(void) const override;
    float getResourceRate(std::string rname) const override;
    float getSwath(void) const { return m_swath; }

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

#endif /* INSTRUMENT_HPP */
