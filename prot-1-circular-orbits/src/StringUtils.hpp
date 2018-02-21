/***********************************************************************************************//**
 *  String utilities.
 *  @class      StringUtils
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mmm-00
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include "prot.hpp"

#include <string>
#include <typeinfo>

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>

class StringUtils
{
public:
    static std::string typeDemangle(const char *name);
    static std::string trimLeft(std::string s);
    static std::string trimRight(std::string s);
    static std::string trim(std::string s);
    static std::vector<std::string> split(const std::string &s, char delim);

    template<typename T>
    static std::string intToHex(T i, bool with_prefix = false);

    static bool isJSON(std::string s);
    static bool isJSONWithSchema(std::string s, std::string schema);

private:
    template<typename Out>
    static void split(const std::string &s, char delim, Out result);
};


/*  This function, credit to Stack Overflow user Kornel Kisielewicz.
 *  See https://stackoverflow.com/a/5100745/1876268
 **/
template<typename T>
std::string StringUtils::intToHex(T i, bool with_prefix)
{
    std::stringstream ss;
    std::string prefix = "";
    if(with_prefix) {
        prefix = "0x";
    }
    ss << prefix << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << i;
    return ss.str();
}

#endif
