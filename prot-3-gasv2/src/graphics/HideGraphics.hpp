/***********************************************************************************************//**
 *  Graphics that can be hidden.
 *  @class      HideGraphics
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-apr-16
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef HIDE_GRAPHICS_HPP
#define HIDE_GRAPHICS_HPP

#include "prot.hpp"

class HideGraphics
{
public:
    HideGraphics(void);
    virtual ~HideGraphics(void) = 0;    /* Abstract class. */

    void hide(void) { m_show = false; }
    void show(void) { m_show = true; }
    void toggleShowHide(void) { m_show = !m_show; }
    bool isShown(void) const { return m_show; }
    bool isHidden(void) const { return !m_show; }

protected:
    bool m_show;
};

#endif /* HIDE_GRAPHICS_HPP */
