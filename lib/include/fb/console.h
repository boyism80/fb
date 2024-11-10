#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifdef _WIN32
#include <Windows.h>
#else
#include <locale.h>
#include <sys/ioctl.h>
#endif
#include <stdarg.h>
#include <string>
#include <sstream>
#include <cstring>
#include <optional>
#include <memory>
#include <mutex>
#include <format>
#include <fb/encoding.h>
#include <iostream>
#include <algorithm>
#include <cpp-terminal/iostream.hpp>
#include <cpp-terminal/screen.hpp>
#include <cpp-terminal/cursor.hpp>

#ifdef _WIN32
#define CONSOLE_TITLE "Private kingdom of the wind - http://cshyeon.com"
inline static bool SetConsoleIcon(int id)
{
    auto hwnd = ::GetConsoleWindow();
    if (hwnd == nullptr)
        return false;

    auto icon = ::LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(id));
    if (icon == nullptr)
        return false;

    ::SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
    ::SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
    return true;
}
#endif

namespace fb {

/**
 * @brief      This class describes a console.
 */
class console
{
public:
    /**
     * @brief      This class describes a position_t.
     */
    class position_t
    {
    public:
        uint32_t x = 0;
        uint32_t y = 1;

    public:
        /**
         * @brief      Constructs a new instance.
         */
        position_t() = default;

        /**
         * @brief      Constructs a new instance.
         *
         * @param[in]  x     { parameter_description }
         * @param[in]  y     { parameter_description }
         */
        position_t(uint32_t x, uint32_t y) :
            x(x),
            y(y)
        { }

        /**
         * @brief      Constructs a new instance.
         *
         * @param[in]  r     { parameter_description }
         */
        position_t(const position_t& r) :
            x(r.x),
            y(r.y)
        { }

        /**
         * @brief      Destroys the object.
         */
        ~position_t() = default;
    };

private:
    position_t _position;
    uint32_t   _additional_y = 0;
    std::mutex _mutex;

private:
    /**
     * @brief      Constructs a new instance.
     */
    console() = default;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    console(const console&) = delete;
    /**
     * @brief      Constructs a new instance.
     *
     * @param      <unnamed>  { parameter_description }
     */
    console(console&&) = delete;
    /**
     * @brief      Destroys the object.
     */
    ~console() = default;

public:
    /**
     * @brief      Assignment operator.
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    console& operator= (console&) = delete;
    /**
     * @brief      Assignment operator.
     *
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    console& operator= (const console&) = delete;

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    static bool kubernetes()
    {
        static auto env = std::getenv("KUBERNETES");
        if (env == nullptr)
            return false;

        static auto enabled = (std::strcmp(env, "enabled") == 0);
        return enabled;
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  fmt   The format
     * @param      args  The arguments
     *
     * @tparam     Args  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <class... Args>
    static fb::console& put(const std::string& fmt, Args&&... args)
    {
        auto& ist = console::get();
        auto  _   = std::lock_guard(ist._mutex);

        auto message  = std::vformat(fmt, std::make_format_args(args...));
        auto position = console::position();

        console::cursor(0, position.y);
        std::cout << std::string(console::width(), ' ');

        console::cursor(position.x, position.y);
        std::cout << UTF8(message, PLATFORM::Windows) << std::flush;

        if (kubernetes())
        {
            std::cout << std::endl;
        }
        return ist;
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  fmt   The format
     * @param      args  The arguments
     *
     * @tparam     Args  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <class... Args>
    static fb::console& puts(const std::string& fmt, Args&&... args)
    {
        auto& ist = console::get();
        auto  _   = std::lock_guard(ist._mutex);

        auto message  = std::vformat(fmt, std::make_format_args(args...));
        auto position = console::position();

        console::cursor(0, position.y);
        std::cout << std::string(console::width(), ' ');

        console::cursor(position.x, position.y);
        std::cout << UTF8(message, PLATFORM::Windows) << std::flush;
        console::next();
        return ist;
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  fmt   The format
     * @param      args  The arguments
     *
     * @tparam     Args  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <class... Args>
    static fb::console& render(const std::string& fmt, Args&&... args)
    {
        auto& ist = console::get();
        auto  _   = std::lock_guard(ist._mutex);

        auto message = std::vformat(fmt, std::make_format_args(args...));
        std::cout << UTF8(message, PLATFORM::Windows) << std::flush;

        if (kubernetes())
            std::cout << std::endl;

        return ist;
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  fmt   The format
     * @param      args  The arguments
     *
     * @tparam     Args  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <class... Args>
    static fb::console& comment(const std::string& fmt, Args&&... args)
    {
        auto& ist = console::get();
        auto  _   = std::lock_guard(ist._mutex);

        auto additional_y = ++ist._additional_y;
        auto message      = std::vformat(fmt, std::make_format_args(args...));
        auto position     = console::position();

        console::cursor(0, position.y + additional_y);
        std::cout << std::string(console::width(), ' ');

        console::cursor(position.x, position.y + additional_y);
        std::cout << UTF8(message, PLATFORM::Windows) << std::flush;
        console::clear(message.size() + 1, position.y + additional_y);
        return ist;
    }

public:
    /**
     * @brief      Clears the given position.
     *
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    static fb::console& clear(const position_t& position)
    {
        if (!kubernetes())
        {
            console::cursor(position.x, position.y);
            std::cout << std::string(console::width() - position.x, ' ') << std::flush;
        }
        return console::get();
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  x     { parameter_description }
     * @param[in]  y     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    static fb::console& clear(uint32_t x, uint32_t y)
    {
        return console::clear(console::position_t{x, y});
    }

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    static fb::console& trim()
    {
        auto position = console::position();
        return console::clear(position);
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  width    The width
     * @param[in]  content  The content
     * @param[in]  side     The side
     *
     * @return     { description_of_the_return_value }
     */
    static bool line(uint16_t width, char content, char side = '+')
    {
        if (width < 3)
            return false;

        auto sstream = std::stringstream();
        sstream << side << std::string(width - 2, content) << side;

        auto before = console::position();
        console::put(sstream.str());
        console::position(before);
        return true;
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  width   The width
     * @param[in]  height  The height
     *
     * @return     { description_of_the_return_value }
     */
    static bool box(uint16_t width, uint16_t height)
    {
        if (kubernetes())
            return false;

        if (height < 3)
            return false;

        auto position = console::position();
        auto rows     = 0;
        auto pivot    = position.y;
        console::position(position.x, pivot + rows++);
        console::line(width, '-', '+');
        while (rows < height - 1)
        {
            console::position(position.x, pivot + rows++);
            console::line(width, ' ', '+');
        }
        console::position(position.x, pivot + rows++);
        console::line(width, '-', '+');
        console::position(position.x, pivot);
        return true;
    }

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    static console::position_t position()
    {
        auto& ist = console::get();
        return ist._position;
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     */
    static void position(const console::position_t& position)
    {
        if (kubernetes())
            return;

        auto& ist     = console::get();
        ist._position = position;
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  x     { parameter_description }
     * @param[in]  y     { parameter_description }
     */
    static void position(uint32_t x, uint32_t y)
    {
        console::position(console::position_t(x, y));
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     */
    static void cursor(uint32_t x, uint32_t y)
    {
        console::cursor(console::position_t{x, y});
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     */
    static void cursor(const position_t& position)
    {
        if (kubernetes())
            return;

        std::cout << Term::cursor_move(position.y, position.x) << std::flush;
    }

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    static uint32_t width()
    {
        if (kubernetes())
        {
            return 80;
        }
        else
        {
            return Term::screen_size().columns();
        }
    }

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    static fb::console& next()
    {
        auto& ist      = console::get();
        auto  position = console::position();
        if (!kubernetes())
        {
            console::position(0, position.y + ist._additional_y + 1);
            ist._additional_y = 0;
        }
        else
        {
            std::cout << std::endl;
        }
        return ist;
    }

private:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    static console& get()
    {
        static std::unique_ptr<console> ist;
        static std::once_flag           flag;
        std::call_once(flag, [] {
            ist.reset(new console());
        });
        return *ist;
    }
};

} // namespace fb

#endif // !__CONSOLE_H__