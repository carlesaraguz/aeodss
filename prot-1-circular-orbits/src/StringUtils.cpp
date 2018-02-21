/***********************************************************************************************//**
 *  String utilities.
 *  @class      StringUtils
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-mmm-00
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "StringUtils.hpp"

#ifdef __GNUG__
std::string StringUtils::typeDemangle(const char *name)
{
    int status;
    char * ptr = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    if(ptr != nullptr && status == 0) {
        return std::string(ptr);
    } else {
        return "Unknown";
    }
}
#else
std::string StringUtils::typeDemangle(const char *name)
{
    return "Unknown";
}
#endif

std::string StringUtils::trimLeft(std::string s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

std::string StringUtils::trimRight(std::string s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

std::string StringUtils::trim(std::string s)
{
    return trimLeft(trimRight(s));
}

std::vector<std::string> StringUtils::split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}


/*  Split functions, credit to Stack Overflow user Evan Teran.
 *  See https://stackoverflow.com/a/236803/1876268.
 */
template<typename Out>
void StringUtils::split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

bool StringUtils::isJSON(std::string s)
{
    return isJSONWithSchema(s, "{\"type\":\"object\"}");
}

bool StringUtils::isJSONWithSchema(std::string s_json, std::string schema)
{
    rapidjson::Document doc;
    if(doc.Parse(s_json).HasParseError()) {
        std::cerr << "Unable to check JSON schema. Invalid JSON.";
        return false;
    }
    rapidjson::Document sd;
    if(sd.Parse(schema).HasParseError()) {
        std::cerr << "Unable to check JSON schema. Invalid schema.";
        return false;
    }

    rapidjson::SchemaDocument schdoc(sd);
    rapidjson::SchemaValidator validator(schdoc);
    if(!doc.Accept(validator)) {
        /* Input JSON is invalid according to the schema: */
        rapidjson::StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        std::cerr << "Invalid schema: " << sb.GetString() << "\n";
        std::cerr << "Invalid keyword: " << validator.GetInvalidSchemaKeyword() << "\n";
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        std::cerr << "Invalid document: " << sb.GetString() << std::endl;
        return false;
    }
    return true;
}
