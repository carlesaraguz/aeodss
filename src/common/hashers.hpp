/***********************************************************************************************//**
 *  Hash generatrs and comparison operators for additional types.
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-21
 *  @version    0.1
 *  @copyright  This file is part of a project developed by Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab) at Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef HASHERS_HPP
#define HASHERS_HPP

#include "prot.hpp"

/* For sf::Vector2i ----------------------------------------------------------------------------- */
class Vector2iHash
{
public:
    std::size_t operator()(const sf::Vector2i& v) const {
        int aggregated = v.x + (v.y * Config::world_width * 2);
        return std::hash<int>()(aggregated);
    }
};

#endif /* HASHERS_HPP */
