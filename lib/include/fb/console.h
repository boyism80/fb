#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifdef  _WIN32
#include <Windows.h>
#define CONSOLE_TITLE "Private kingdom of the wind - http://cshyeon.com"
bool SetConsoleIcon(int id);
#else
#include <locale.h>
#endif
#include <stdarg.h>
#include <string>
#include <cstring>
#include <optional>
#include <memory>
#include <mutex>
#include <format>
#include <fb/encoding.h>
#include <cpp-terminal/iostream.hpp>
#include <cpp-terminal/screen.hpp>
#include <cpp-terminal/cursor.hpp>

namespace fb {

class console
{
private:
    uint32_t   _x = 0, _y = 1;
    uint32_t   _additional_y = 0;
    std::mutex _mutex;

private:
    console() = default;

public:
    console(const console&) = delete;
    console(console&&)      = delete;
    ~console()              = default;

public:
    console& operator= (console&)       = delete;
    console& operator= (const console&) = delete;

public:
    template <class... Args>
    fb::console& put(const std::string& fmt, Args&&... args);
    template <class... Args>
    fb::console& puts(const std::string& fmt, Args&&... args);
    template <class... Args>
    fb::console& render(const std::string& fmt, Args&&... args);
    template <class... Args>
    fb::console& comment(const std::string& fmt, Args&&... args);
    fb::console& clear(uint16_t x, uint16_t y);
    fb::console& trim();
    bool         line(uint16_t width, char content, char side = '+');
    bool         box(uint16_t width, uint16_t height);
    uint16_t     x() const;
    void         x(uint16_t val);
    uint16_t     y() const;
    void         y(uint16_t val);
    void         reset_x();
    void         reset_y();
    uint16_t     width() const;
    uint16_t     height() const;
    fb::console& next();
    fb::console& cursor(uint16_t x, uint16_t y);
    void         cursor(uint16_t* x, uint16_t* y) const;

public:
    static console& get();
};

} // namespace fb

template <class... Args>
fb::console& fb::console::put(const std::string& fmt, Args&&... args)
{
    auto _       = std::lock_guard(this->_mutex);

    auto message = std::vformat(fmt, std::make_format_args(args...));
    Term::cout << Term::cursor_move(this->_y, 0) << std::string(this->width(), ' ') << Term::cursor_move(this->_y, this->_x) << UTF8(message, PLATFORM::Windows) << std::flush;
    return *this;
}

template <class... Args>
fb::console& fb::console::puts(const std::string& fmt, Args&&... args)
{
    auto _       = std::lock_guard(this->_mutex);

    auto message = std::vformat(fmt, std::make_format_args(args...));
    Term::cout << Term::cursor_move(this->_y, 0) << std::string(this->width(), ' ') << Term::cursor_move(this->_y, this->_x) << UTF8(message, PLATFORM::Windows) << std::flush;

    this->next();
    return *this;
}

template <class... Args>
fb::console& fb::console::render(const std::string& fmt, Args&&... args)
{
    auto _       = std::lock_guard(this->_mutex);

    auto message = std::vformat(fmt, std::make_format_args(args...));
    Term::cout << Term::cursor_move(this->_y, this->_x) << UTF8(message, PLATFORM::Windows) << std::flush;

    return *this;
}

template <class... Args>
fb::console& fb::console::comment(const std::string& fmt, Args&&... args)
{
    auto _            = std::lock_guard(this->_mutex);
    auto additional_y = ++this->_additional_y;
    auto message      = std::vformat(fmt, std::make_format_args(args...));
    Term::cout << Term::cursor_move(this->_y + additional_y, 0) << std::string(this->width(), ' ') << Term::cursor_move(this->_y + additional_y, this->_x)
               << UTF8(message, PLATFORM::Windows) << std::flush;
    this->clear(message.size(), this->_y + additional_y);

    return *this;
}

#endif // !__CONSOLE_H__