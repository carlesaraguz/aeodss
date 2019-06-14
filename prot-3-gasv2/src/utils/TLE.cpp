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

#include "TLE.hpp"

CREATE_LOGGER(TLE)

TLE::TLE(void)
    : sat_number(0)
    , inter_des_year(0)
    , inter_des_num_year(0)
    , epoch_year(0)
    , epoch_doy(0.0)
    , first_time(0.0)
    , bstar(0.0)
    , tle_number(0)
    , mean_anomaly(0.0)
    , mean_motion(0.0)
    , revolutions(0)
    , second_time(0.0)
    , m_tle_line_0("")
    , m_tle_line_1("")
    , m_tle_line_2("")
{ }

TLE::TLE(std::string tle)
    : TLE()
{
    std::stringstream ss(tle);
    int line_count = 0;
    std::string line, tle0, tle1, tle2;
    for(; std::getline(ss, line) && line_count <= 3; line_count++) {
        switch(line_count) {
            case 0:
                tle0 = Utils::trim(line);
                break;
            case 1:
                tle1 = Utils::trim(line);
                break;
            case 2:
                tle2 = Utils::trim(line);
                break;
        }
    }
    setTLE(tle0, tle1, tle2);
}

TLE::TLE(std::string tle0, std::string tle1, std::string tle2)
{
    tle0 = Utils::trim(tle0);
    tle1 = Utils::trim(tle1);
    tle2 = Utils::trim(tle2);
    setTLE(tle0, tle1, tle2);
}

bool TLE::operator==(const TLE& other) const
{
    return (sat_name == other.sat_name &&
        sat_number == other.sat_number &&
        classification == other.classification &&
        inter_des_year == other.inter_des_year &&
        inter_des_num_year == other.inter_des_num_year &&
        inter_des_piece == other.inter_des_piece &&
        epoch_year == other.epoch_year &&
        epoch_doy == other.epoch_doy &&
        first_time == other.first_time &&
        second_time == other.second_time &&
        bstar == other.bstar &&
        tle_number == other.tle_number &&
        orbit_params.ecc == other.orbit_params.ecc &&
        orbit_params.inc == other.orbit_params.inc &&
        orbit_params.raan == other.orbit_params.raan &&
        orbit_params.argp == other.orbit_params.argp &&
        orbit_params.sma == other.orbit_params.sma &&
        orbit_params.mean_motion == other.orbit_params.mean_motion &&
        mean_anomaly == other.mean_anomaly &&
        mean_motion == other.mean_motion &&
        revolutions == other.revolutions);
}

bool TLE::operator!=(const TLE& other) const
{
    return !(*this == other);
}

void TLE::setTLE(std::string tle0, std::string tle1, std::string tle2)
{
    m_tle_line_0 = Utils::trim(tle0);
    m_tle_line_1 = tle1;
    m_tle_line_2 = tle2;

    if(m_tle_line_0.length() > 0 && m_tle_line_1.length() > 0 && m_tle_line_2.length() > 0) {
        sat_name = m_tle_line_0;
        if(!parseLine1(m_tle_line_1) || !parseLine2(m_tle_line_2)) {
            throw std::runtime_error("Bad TLE format: parsing failed.");
        }
    } else {
        m_tle_line_0 = "";
        m_tle_line_1 = "";
        m_tle_line_2 = "";
        throw std::runtime_error("Bad TLE format: a TLE must be composed of the satellite name and two lines.");
    }
}

bool TLE::parseLine1(std::string line)
{
    std::smatch matched_values;      /* output with the different matches */
    int exponent;
    bool correct_format = true;
    int checksum = 0;

    /* Format rule of the first line in the TLE */
    std::string rule = "1 ([0-9 ]{5})([A-Z]) ([0-9 ]{2})([0-9]{3})([A-Z ]{3}) ";
    rule += "([0-9]{2})([0-9 \\.]{12}) ([- ].[0-9\\.]{8}) ";
    rule += "([- ][0-9-]{5})([-+][0-9]) ([- ][0-9-]{5})([-+][0-9]) ";
    rule += "0 ([0-9 ]{4})([0-9])";
    std::regex r_rule(rule);
    std::regex_search(line, matched_values, r_rule);

    if(matched_values.size() == 15) {
        /* Verify Checksum */
        for(unsigned int i = 0; i < (line.length() - 1); i ++) {
            try {
                checksum += std::stoi(std::string(1, line[i]));
            } catch(const std::invalid_argument& ia) {
                if(line[i] == '-') {
                    checksum++;
                }
            }
        }
        if((checksum % 10) != std::stoi(matched_values[14])) {
            Log::err << "TLE parsing failed (line 1): the TLE has an error! The computed checksum ("
                << (checksum % 10) << ") is different than the reference (" << std::stoi(matched_values[14]) << ").\n";
            correct_format = false;
        }
        sat_number = std::stoi(matched_values[1]);          /* 1: Satellite number */
        classification = matched_values[2];                 /* 2: Classification */
        inter_des_year = std::stoi(matched_values[3]);      /* 3: International Designation 2 digits of launch Year */
        inter_des_num_year = std::stoi(matched_values[4]);  /* 4: International Designation launch number */
        inter_des_piece = matched_values[5];                /* 5: International Designation piece of launch */
        epoch_year = std::stoi(matched_values[6]);          /* 6: Epoch year */
        epoch_doy = std::stod(matched_values[7]);           /* 7: Epoch day of year */
        first_time = std::stod(matched_values[8]) * 2;      /* 8: First Time derivative of Mean motion */
        second_time = std::stod(matched_values[9]);         /* 9: Second Time derivative of Mean motion */
        exponent = std::stoi(matched_values[10]);           /* 10: Exponent (scientific notation). */
        second_time = second_time * std::pow(10, exponent) * 6;
        bstar = std::stod(matched_values[11]);              /* 11: BSTAR */
        exponent = std::stoi(matched_values[12]);           /* 12: Exponent (scientific notation). */
        bstar = bstar * std::pow(10, exponent);
        tle_number = std::stod(matched_values[13]);         /* 13: TLE number */
    } else {
        Log::err << "TLE parsing failed (line 1), unexpected format: " << line << "\n";
        correct_format = false;
    }
    return correct_format;
}

bool TLE::parseLine2(std::string line)
{
    std::smatch matched_values;      /* output with the different matches */
    double period, value;
    bool correct_format = true;
    int checksum = 0;

    /* Format rule of the first line in the TLE */
    std::string rule = "2 ([0-9 ]{5}) ([- ][0-9 \\.]{7}) ([0-9 \\.]{8}) ([0-9]{7}) ";
    rule += "([0-9 \\.]{8}) ([0-9 \\.]{8}) ([0-9 \\.]{11})([0-9 ]{5})([0-9])";
    std::regex r_rule(rule);
    std::regex_search(line, matched_values, r_rule);
    if(matched_values.size() == 10) {
        /* Verify Checksum */
        for(unsigned int i = 0; i < (line.length() - 1); i ++) {
            try {
                checksum += std::stoi(std::string(1, line[i]));
            } catch(const std::invalid_argument& ia) {
                if(line[i] == '-') {
                    checksum ++;
                }
            }
        }
        if((checksum % 10) != std::stoi(matched_values[9])) {
            Log::err << "TLE parsing failed (line 2): the TLE has an error! The computed checksum ("
                << (checksum % 10) << ") is different than the reference (" << std::stoi(matched_values[9]) << ").\n";
            correct_format = false;
        }

        /* Satellite number */
        sat_number = std::stoi(matched_values[1]);

        /* Inclination */
        orbit_params.inc = std::stod(matched_values[2]);
        if(orbit_params.inc < 0.0 || orbit_params.inc > 360.0){
            Log::err << "TLE parsing failed (line 2): inclination out of range [0, 360] degrees.\n";
            correct_format = false;
        }
        /* RAAN */
        orbit_params.raan = std::stod(matched_values[3]);
        if(orbit_params.raan < 0.0 || orbit_params.raan > 360.0){
            Log::err << "TLE parsing failed (line 2): right ascension of the ascending node out of range [0, 360] degrees.\n";
            correct_format = false;
        }
        /* Eccentricity */
        orbit_params.ecc = std::stod(matched_values[4]) / 10000000.0;
        if(orbit_params.ecc < 0.0 || orbit_params.ecc >= 1.0) {
            Log::err << "TLE parsing failed (line 2): eccentricity out of range [0, 1).\n";
            correct_format = false;
        }
        /* Argument of perigee */
        orbit_params.argp = std::stod(matched_values[5]);
        if(orbit_params.argp < 0.0 || orbit_params.argp > 360.0) {
            Log::err << "TLE parsing failed (line 2): argument of perigee out of range [0, 360] degrees.\n";
            correct_format = false;
        }
        /* Mean anomaly */
        mean_anomaly = std::stod(matched_values[6]);
        if(mean_anomaly < 0.0 || mean_anomaly > 360.0) {
            Log::err << "TLE parsing failed (line 2): mean anomaly out of range [0, 360] degrees.\n";
            correct_format = false;
        }
        /* Mean motion */
        mean_motion = std::stod(matched_values[7]);
        if(mean_motion == 0.0) {
            Log::err << "TLE parsing failed (line 2): mean motion cannot be zero.\n";
            correct_format = false;
        }
        orbit_params.mean_motion = mean_motion;

        /* Revolution number at epoch */
        revolutions = std::stod(matched_values[8]);

        /* Semi major axis */
        period = 86400.0 / mean_motion;
        value = std::pow(period / (2 * Config::pi), 2);
        orbit_params.sma = std::pow(Config::earth_mu * value, (1.0 / 3));
        orbit_params.sma = floor(orbit_params.sma * 1000) / 1000; /* Flooring until mm */
        if(orbit_params.sma < Config::earth_radius){
            Log::err << "TLE parsing failed (line 2): semi-major axis cannot be smaller than the Earth radius.\n";
            correct_format = false;
        }
    } else {
        Log::err << "TLE parsing failed (line 2), unexpected format: " << line << "\n";
        correct_format = false;
    }
    return correct_format;
}
