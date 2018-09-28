/***********************************************************************************************//**
 *  Miscellaneous utilities.
 *  @class      Utils
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-26
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Utils.hpp"

std::chrono::time_point<std::chrono::steady_clock> Utils::m_time_point;

void Utils::tic(void)
{
    m_time_point = std::chrono::steady_clock::now();
}

float Utils::toc(void)
{
    std::chrono::duration<float> diff = std::chrono::steady_clock::now() - m_time_point;
    m_time_point = std::chrono::steady_clock::now();
    return diff.count();
}

std::string Utils::trimLeft(std::string s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

std::string Utils::trimRight(std::string s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

std::string Utils::trim(std::string s)
{
    return trimLeft(trimRight(s));
}

std::vector<std::string> Utils::split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

/*  Split functions, credit to Stack Overflow user Evan Teran.
 *  See https://stackoverflow.com/a/236803/1876268.
 */
template<typename Out>
void Utils::split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}
