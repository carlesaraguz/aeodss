/***********************************************************************************************//**
 *  Logging and console message utils.
 *  @class      Log
 *  @authors    Carles Araguz (CA), carles.araguz@upc.edu
 *  @date       2018-sep-13
 *  @version    0.1
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Technical University of Catalonia - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef LOG_HPP
#define LOG_HPP

#include "prot.hpp"
#include <cassert>
#include <ostream>
#include "WallTime.hpp"
#include "VirtualTime.hpp"

class LogStream : public std::streambuf
{
public:
    enum class Color {
        RED_BRIGHT,
        RED_DARK,
        BLUE_BRIGHT,
        BLUE_DARK,
        YELLOW_BRIGHT,
        YELLOW_DARK,
        GREEN_BRIGHT,
        GREEN_DARK,
        PURPLE_BRIGHT,
        PURPLE_DARK,
        WHITE_BG,
        GRAY,
        NO_COLOR
    };

    enum class Level {
        NONE,
        DEBUG,
        WARNING,
        ERROR
    };

    LogStream(void);
    LogStream(std::basic_ostream<char>& out, Level level, std::string cname, char c, Color icon_color, Color text_color = Color::NO_COLOR);

    /* Non-copiable. */
    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

    void setColors(Color icon_color, Color text_color = Color::NO_COLOR);
    void setIcon(char c) { m_icon = c; }
    static void setNameLength(int l);
    static void setLogLevel(Level l) { m_enabled_level = l; }

protected:
    int_type overflow(int_type c = traits_type::eof());
    std::streamsize xsputn(const char* s, std::streamsize n);

private:
    static Level m_enabled_level;
    Level m_level;
    Color m_color_icon;
    Color m_color_text;
    char m_icon;
    std::string m_cname;
    static std::map<Color, const std::string> m_color_lut;
    static int m_max_cname_len;

    std::basic_ostream<char>& m_out;
    bool m_new_line;

    void conditionalPrintHeader(void);
    void endLine(void);
};

#define CREATE_LOGGER(klass)                        \
    namespace Log {                                 \
        static std::ostream dbg(new LogStream(      \
            std::clog,                              \
            LogStream::Level::DEBUG,                \
            #klass,                                 \
            '>',                                    \
            LogStream::Color::BLUE_BRIGHT,          \
            LogStream::Color::NO_COLOR));           \
        static std::ostream err(new LogStream(      \
            std::clog,                              \
            LogStream::Level::ERROR,                \
            #klass,                                 \
            'e',                                    \
            LogStream::Color::RED_BRIGHT,           \
            LogStream::Color::RED_DARK));           \
        static std::ostream warn(new LogStream(     \
            std::clog,                              \
            LogStream::Level::WARNING,              \
            #klass,                                 \
            '!',                                    \
            LogStream::Color::YELLOW_BRIGHT,        \
            LogStream::Color::YELLOW_DARK));        \
    }


#endif /* LOG_HPP */
