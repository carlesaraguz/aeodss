/***********************************************************************************************//**
 *  Box with message.
 *  @class      MessageBox
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2019-apr-22
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "MessageBox.hpp"

MessageBox::MessageBox(void)
    : m_margin(5.f)
{
    m_box.setFillColor(sf::Color(0, 0, 0, 127));
    m_box.setOutlineColor(sf::Color::White);
    m_box.setOutlineThickness(1.f);

    m_txt.setFont(Config::fnt_monospace);
    m_txt.setCharacterSize(12);
    m_txt.setFillColor(sf::Color::White);
    m_txt.setPosition(m_margin, m_margin - 5.f);
    setMessage("Undefined");
}

void MessageBox::setMargin(float m)
{
    m_margin = m;
    m_txt.setPosition(m_margin, m_margin - 5.f);
    setMessage(m_msg);
}

void MessageBox::setMessage(std::string msg)
{
    m_msg = msg;
    m_txt.setString(m_msg);
    float txt_w = m_txt.getLocalBounds().width;
    float txt_h = m_txt.getLocalBounds().height;
    m_box.setSize({txt_w + 2.f * m_margin, txt_h + 2.f * m_margin});
}

void MessageBox::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    target.draw(m_box, states);
    target.draw(m_txt, states);
}
