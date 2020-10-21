#ifndef __CONSOLE_H__
#define	__CONSOLE_H__

#ifdef  _WIN32
#include <Windows.h>
#define	CONSOLE_TITLE	"Private kingdom of the wind - http://cshyeon.com"
bool SetConsoleIcon(int id);
#else
#include <ncursesw/curses.h>
#endif
#include <stdarg.h>
#include <string>
#include <cstring>
#include <optional>
#include "module/encoding/encoding.h"

namespace fb {

class console
{
private:
    static console*         _ist;

private:
#ifdef _WIN32
    HANDLE                  _stdout;
#endif
    std::optional<uint16_t> _x, _y;
    uint16_t                _width, _height;
    uint16_t                _current_line;

private:
    console();
    ~console();

private:
    std::string             format(const std::string& f, va_list* args);

public:
    fb::console&            move(uint16_t x, uint16_t y);
    uint32_t                puts(const char* format, ...);
    bool                    clear(uint16_t x, uint16_t y);
    bool                    line(uint16_t x, uint16_t y, uint16_t width, char content, char side = '+');
    bool                    box(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    uint16_t                width() const;
    uint16_t                height() const;
    uint32_t                current_line() const;
    fb::console&            current_line(uint32_t row);
    fb::console&            next();

public:
    static console&         get();
    static void             release();
};

}

#endif // !__CONSOLE_H__