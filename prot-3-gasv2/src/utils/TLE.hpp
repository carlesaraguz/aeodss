/***********************************************************************************************//**
 *  Two-Line Element set parser.
 *  @class      TLE
 *  @authors    Joan Adrià Ruiz de Azúa (JRA), joan.adria@tsc.upc.edu
 *  @date       2017-dec-06
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 *  @note       TLE class has been ported from dss-sim project.
 **************************************************************************************************/

#ifndef TLE_HPP
#define TLE_HPP

#include "prot.hpp"
#include <regex>
#include "Utils.hpp"
#include "AgentMotion.hpp"  /* Provides OrbitalParams struct. */

/***********************************************************************************************//**
 * Two-Line Element representation. Class that maps the standard TLE of an orbiting object
 * (typically a Satellite). More information can be found in
 * <a href="https://en.wikipedia.org/wiki/Two-line_element_set"> this page </a>
 **************************************************************************************************/
class TLE
{
public:
    std::string sat_name;               /**< Satellite name. */
    int sat_number;                     /**< Satellite Identifier. */
    std::string classification;         /**< Type of Satellite classification. */
    int inter_des_year;                 /**< Last two digits of launch year. */
    int inter_des_num_year;             /**< The Launch number in that year. */
    std::string inter_des_piece;        /**< The piece of the launcher. */
    int epoch_year;                     /**< Last two digits of the TLE year. */
    double epoch_doy;                   /**< Day and fractional day of the TLE. */
    double first_time;                  /**< First time derivative of the mean motion [d(deg)/dt]. */
    double second_time;                 /**< Second time derivative of the mean motion [d²(deg)/dt]. */
    double bstar;                       /**< BSTAR to compute aerodynamic drag. */
    int tle_number;                     /**< TLE identifier. */
    OrbitalParams orbit_params;         /**< Orbital parameters of the Satellite. */
    double mean_anomaly;                /**< Satellite position - mean anomaly [deg]. */
    double mean_motion;                 /**< Satellite mean motion [rev/day]. */
    int revolutions;                    /**< Revolutions that the Satellite has currently done. */

    /*******************************************************************************************//**
     * Default contructor of a TLE. It initialize all the items at 0.0 value or empty strings.
     **********************************************************************************************/
    TLE(void);

    /*******************************************************************************************//**
     *  Creates and parses TLE data provided in a single string. The format of that string expects,
     *  at least, three lines of data. The format of that string should be the one defined in the
     *  Two-Line Elements standard. This constructor relies on the operations and parsing of setTLE.
     *  @param  tle   TLE in string format, with lines separated by the Line-Feed ('\n') character.
     *  @see    TLE::setTLE
     **********************************************************************************************/
    TLE(std::string tle);

    /*******************************************************************************************//**
     *  Creates and parses TLE data provided in three separate strings. The format of each string
     *  should be the one defined in the Two-Line Elements standard. Each individual line is trimmed
     *  to remove leading and trailing whitespace (including LF, tabs and any other whitespace
     *  character) before parsing. This constructor relies on the operations and parsing of setTLE.
     *  @param  tle0    Initial line of a TLE: satellite identifier and name.
     *  @param  tle1    First TLE line.
     *  @param  tle2    Second TLE line.
     *  @see    TLE::setTLE
     **********************************************************************************************/
    TLE(std::string tle0, std::string tle1, std::string tle2);

    /*******************************************************************************************//**
     *  Auto-generated default destructor.
     **********************************************************************************************/
    ~TLE(void) = default;

    /*******************************************************************************************//**
     *  Copy-construct a TLE from an existing object. Auto-generated.
     **********************************************************************************************/
    TLE(const TLE& other) = default;

    /*******************************************************************************************//**
     *  Auto-generated copy constructor.
     **********************************************************************************************/
    TLE& operator=(const TLE& other) = default;

    /*******************************************************************************************//**
     *  Set TLE values from an TLE in string format in three strings that correspond to each line.
     *  @param  tle0    Initial line of a TLE: satellite identifier and name. Will be trimmed to
     *                  remove leading and/or ltrailing whitespace. This is not an optional
     *                  parameter and cannot be empty.
     *  @param  tle1    First TLE line.
     *  @param  tle2    Second TLE line.
     *  @throws runtime_error If either of the arguments are empty or could not be parsed.
     **********************************************************************************************/
    void setTLE(std::string tle0, std::string tle1, std::string tle2);

    /*******************************************************************************************//**
     *  Equal operator. Performs element-by-element comparison (including strings.)
     *  @returns    True if all the TLE members are the same, false otherwise.
     **********************************************************************************************/
    bool operator==(const TLE& other) const;

    /*******************************************************************************************//**
     *  Not-equal operator. Performs element-by-element comparison (including strings.)
     *  @returns    True if one or more elements of the TLE objects are different. False otherwise.
     **********************************************************************************************/
    bool operator!=(const TLE& other) const;

    /*******************************************************************************************//**
     *  Getter for the initial TLE line.
     *  @returns    The initial TLE with the satellite name.
     *  @todo       Regenerate the TLE line based on the public fields. (TODO)
     **********************************************************************************************/
    std::string getLine0(void) const { return m_tle_line_0; }

    /*******************************************************************************************//**
     *  Getter for the first elements line.
     *  @returns    The TLE line with the first set of elements.
     *  @todo       Regenerate the TLE line based on the public fields. (TODO)
     **********************************************************************************************/
    std::string getLine1(void) const { return m_tle_line_1; }

    /*******************************************************************************************//**
     *  Getter for the second elements line.
     *  @returns    The TLE line with the second set of elements.
     *  @todo       Regenerate the TLE line based on the public fields. (TODO)
     **********************************************************************************************/
    std::string getLine2(void) const { return m_tle_line_2; }

private:
    std::string m_tle_line_0;   /**< TLE initial line (with satellite name). */
    std::string m_tle_line_1;   /**< TLE first elements line. */
    std::string m_tle_line_2;   /**< TLE second elements line. */

    /*******************************************************************************************//**
     * Parses the first line of a TLE in text format.
     *  @param      line    First line of a TLE in text format.
     *  @returns    True if the parsing succeeded. False if the TLE line was not provided in the
     *              expected format or its parsing did not succeed.
     **********************************************************************************************/
    bool parseLine1(std::string line);

    /*******************************************************************************************//**
     *  Parses the second line of a TLE in text format.
     *  @param      line    Second line of a TLE in text format.
     *  @returns    True if the parsing succeeded. False if the TLE line was not provided in the
     *              expected format or its parsing did not succeed.
     **********************************************************************************************/
    bool parseLine2(std::string line);
};


#endif /* TLE_HPP */
