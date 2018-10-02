/***********************************************************************************************//**
 *  Miscellaneous utilities.
 *  @class      Utils
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-26
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef UTILS_HPP
#define UTILS_HPP

#include "prot.hpp"

class Utils
{
public:
    static void tic(void);
    static float toc(void);

    static std::string trimLeft(std::string s);
    static std::string trimRight(std::string s);
    static std::string trim(std::string s);
    static std::vector<std::string> split(const std::string &s, char delim);

    template<typename T>
    static std::string intToHex(T i, bool with_prefix = false);

private:
    static std::chrono::time_point<std::chrono::steady_clock> m_time_point;

    template<typename Out>
    static void split(const std::string &s, char delim, Out result);
};

/*  This function, credit to Stack Overflow user Kornel Kisielewicz.
*  See https://stackoverflow.com/a/5100745/1876268
**/
template<typename T>
std::string Utils::intToHex(T i, bool with_prefix)
{
    std::stringstream ss;
    std::string prefix = "";
    if(with_prefix) {
        prefix = "0x";
    }
    ss << prefix << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << i;
    return ss.str();
}

#endif /* UTILS_HPP */
