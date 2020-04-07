/***********************************************************************************************//**
 *  Miscellaneous utilities.
 *  @class      Utils
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-26
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef UTILS_HPP
#define UTILS_HPP

#include "prot.hpp"
#include <typeinfo>

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

class Utils
{
public:
    static void tic(void);
    static float toc(void);

    static std::string trimLeft(std::string s);
    static std::string trimRight(std::string s);
    static std::string trim(std::string s);
    static void removeWhitespace(std::string& s);
    static std::vector<std::string> split(const std::string &s, char delim);

    template <typename T>
    static std::string intToHex(T i, bool with_prefix = false);

    static std::string typeDemangle(const char *name);

    template <class VecType, typename Integral>
    static void safeI(Integral& i, VecType& v);

    template <class VecType, typename Integral>
    static void safeXY(Integral& x, Integral& y, VecType& v);

private:
    static std::chrono::time_point<std::chrono::steady_clock> m_time_point;

    template<typename Out>
    static void split(const std::string &s, char delim, Out result);

    static void logErrProxy(std::string msg);
};

/*  This function, credit to Stack Overflow user Kornel Kisielewicz.
*  See https://stackoverflow.com/a/5100745/1876268
**/
template <typename T>
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


template <class VecType, typename Integral>
void Utils::safeI(Integral& i, VecType& v)
{
    Integral len = v.size();
    if(len > 0) {
        if(i >= len) {
            i = len - 1;
        }
        if(i < 0) {
            i = 0;
        }
    } else {
        /* There's nothing we can do here other than failing with more information than std::out_of_range. */
        std::stringstream ss;
        ss << "Trying to access a 1-d container of type \'" << typeDemangle(typeid(VecType).name()) << "\' that is empty. ";
        ss << "Index is of type \'" << typeid(i).name() << "\' and its value is: " << i << ".\n";
        logErrProxy(ss.str());
        throw std::out_of_range("Trying to access an empty container.");
    }
}

template <class VecType, typename Integral>
void Utils::safeXY(Integral& x, Integral& y, VecType& v)
{
    Integral len1 = v.size();
    if(len1 > 0) {
        if(x >= len1) {
            x = len1 - 1;
        }
        if(x < 0) {
            x = 0;
        }
        Integral len2 = v.at(x).size();
        if(len2 > 0) {
            if(y >= len2) {
                y = len2 - 1;
            }
            if(y < 0) {
                y = 0;
            }
        } else {
            /* There's nothing we can do here other than failing with more information than std::out_of_range. */
            std::stringstream ss;
            ss << "Trying to access an inner element of a 2-d container of type \'" << typeDemangle(typeid(v).name()) << "\' that is empty. ";
            ss << "Indices are of types (\'" << typeid(x).name() << "\', \'" << typeid(y).name() << "\') and their values are: (" << x << ", " << y << ").\n";
            ss << "Size of the outer container is: " << len1 << ".\n";
            logErrProxy(ss.str());
            throw std::out_of_range("Trying to access an empty container element.");
        }
    } else {
        /* There's nothing we can do here other than failing with more information than std::out_of_range. */
        std::stringstream ss;
        ss << "Trying to access a 2-d container of type \'" << typeDemangle(typeid(v).name()) << "\' that is empty. ";
        ss << "Indices are of types (\'" << typeid(x).name() << "\', \'" << typeid(y).name() << "\') and their values are: (" << x << ", " << y << ").\n";
        logErrProxy(ss.str());
        throw std::out_of_range("Trying to access an empty container.");
    }
}

#endif /* UTILS_HPP */
