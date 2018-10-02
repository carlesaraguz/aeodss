/***********************************************************************************************//**
 *  Interface for classes that can be graphically represented.
 *  @class      HasView
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-20
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef HAS_VIEW_HPP
#define HAS_VIEW_HPP

#include "prot.hpp"

class HasView
{
public:
    virtual const sf::Drawable& getView(void) const = 0;
};

#endif /* HAS_VIEW_HPP */
