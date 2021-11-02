#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifdef  _WIN32
#include <Windows.h>
#define CONSOLE_TITLE   "Private kingdom of the wind - http://cshyeon.com"
bool SetConsoleIcon(int id);
#else
#include <locale.h>
#include <ncursesw/curses.h>
#endif
#include <stdarg.h>
#include <string>
#include <cstring>
#include <optional>
#include <memory>
#include <mutex>
#include "module/encoding/encoding.h"

namespace fb {

class console
{
private:
    static std::unique_ptr<console>         _ist;

private:
#ifdef _WIN32
    HANDLE                  _stdout;
    int                     _x       = 0;
    int                     _y       = 0;
#endif
    uint16_t                _width   = 0;
    uint16_t                _height  = 0;

private:
    console();

public:
    console(const console&) = delete;
    console(console&&) = delete;
    ~console();

public:
    console& operator = (console&) = delete;
    console& operator = (const console&) = delete;

private:
    std::string             format(const std::string& f, va_list* args);

public:
    fb::console&            put(const char* format, ...);
    fb::console&            puts(const char* format, ...);
    fb::console&            clear(uint16_t x, uint16_t y);
    fb::console&            trim();
    bool                    line(uint16_t x, uint16_t y, uint16_t width, char content, char side = '+');
    bool                    box(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    uint16_t                x() const;
    uint16_t                y() const;
    uint16_t                width() const;
    uint16_t                height() const;
    fb::console&            next();
    fb::console&            cursor(uint16_t x, uint16_t y);
    void                    cursor(uint16_t* x, uint16_t* y) const;

public:
    static console&         get();
};

}

#endif // !__CONSOLE_H__