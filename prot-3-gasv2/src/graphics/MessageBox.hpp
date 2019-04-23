/***********************************************************************************************//**
 *  Box with message.
 *  @class      MessageBox
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2019-apr-22
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef MESSAGE_BOX_HPP
#define MESSAGE_BOX_HPP

#include "prot.hpp"

class MessageBox : public sf::Transformable, public sf::Drawable
{
public:
    MessageBox(void);

    void setMessage(std::string msg);
    float getWidth(void) const { return m_box.getSize().x; }
    float getHeight(void) const { return m_box.getSize().y; }
    sf::Vector2f getSize(void) const { return m_box.getSize(); }
    void setMargin(float m);

private:
    std::string m_msg;
    sf::RectangleShape m_box;
    sf::Text m_txt;
    float m_margin;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif /* MESSAGE_BOX_HPP */
