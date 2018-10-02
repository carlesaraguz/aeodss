/***********************************************************************************************//**
 *  Logging and console message utils.
 *  @class      Log
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-13
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#include "Log.hpp"

int LogStream::m_max_cname_len = 0;

std::map<LogStream::Color, const std::string> LogStream::m_color_lut = {
    { Color::RED_DARK     , "\x1b[0;31m" },
    { Color::RED_BRIGHT   , "\x1b[1;31m" },
    { Color::GREEN_DARK   , "\x1b[0;32m" },
    { Color::GREEN_BRIGHT , "\x1b[1;32m" },
    { Color::YELLOW_DARK  , "\x1b[0;33m" },
    { Color::YELLOW_BRIGHT, "\x1b[1;33m" },
    { Color::BLUE_DARK    , "\x1b[0;34m" },
    { Color::BLUE_BRIGHT  , "\x1b[1;34m" },
    { Color::PURPLE_DARK  , "\x1b[0;35m" },
    { Color::PURPLE_BRIGHT, "\x1b[1;35m" },
    { Color::WHITE_BG     , "\x1b[30;47m" },
    { Color::GRAY         , "\x1b[1;30m" },
    { Color::NO_COLOR     , "\x1b[0m" }
};

LogStream::LogStream(void)
    : LogStream(std::clog, "UNDEFINED", '#', Color::NO_COLOR)
{ }

LogStream::LogStream(std::basic_ostream<char>& out, std::string cname, char c, Color icon_color, Color text_color)
    : m_out(out)
    , m_new_line(true)
    , m_icon(c)
    , m_color_icon(icon_color)
    , m_color_text(text_color)
    , m_cname(cname)
{
    setNameLength(cname.length());
}

LogStream::int_type LogStream::overflow(int_type c)
{
    if(c == traits_type::eof()) {
        return traits_type::eof();
    } else {
        char_type ch = traits_type::to_char_type(c);
        return xsputn(&ch, 1) == 1 ? c : traits_type::eof();
    }
}

std::streamsize LogStream::xsputn(const char* s, std::streamsize n)
{
    conditionalPrintHeader();
    std::string str;
    for(int c = 0; c < n; c++) {
        str += s[c];
    }
    m_out << str;
    if(s[n - 1] == '\n') {
        m_new_line = true;
        endLine();
    }
    return n;
}

void LogStream::conditionalPrintHeader(void)
{
    if(m_new_line) {
        m_out << m_color_lut[Color::GRAY] << "[ " << m_color_lut[Color::NO_COLOR]
            << WallTime::getTimeStr();
        m_out << m_color_lut[Color::GRAY] << " |" << m_color_lut[Color::NO_COLOR] << " "
            << std::setw(7) << std::setprecision(1) << std::fixed << VirtualTime::now();
        m_out << m_color_lut[Color::GRAY] << " |" << m_color_lut[Color::NO_COLOR] << " "
            << std::right << std::setfill(' ') << std::setw(m_max_cname_len) << m_cname;
        m_out << m_color_lut[Color::GRAY] << " ]" << m_color_lut[Color::NO_COLOR] << " ("
            << m_color_lut[m_color_icon] << m_icon << m_color_lut[Color::NO_COLOR] << ") ";
        m_out << m_color_lut[m_color_text];
        m_new_line = false;
    }
}

void LogStream::endLine(void)
{
    m_out << m_color_lut[Color::NO_COLOR];
}

void LogStream::setColors(Color icon_color, Color text_color)
{
    m_color_icon = icon_color;
    m_color_text = text_color;
}

void LogStream::setNameLength(int l)
{
    if(l > m_max_cname_len) {
        m_max_cname_len = l;
    }
}
