#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifdef _WIN32
#include <io.h>
#else
#include <locale.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ncursesw/curses.h>
#endif
#include <iostream>
#include <stdarg.h>
#include <sstream>
#include <cstring>
#include <mutex>
#include <format>

#ifdef _WIN32
#define CONSOLE_TITLE "Private kingdom of the wind - http://cshyeon.com"
bool SetConsoleIcon(int id);
#endif

namespace fb {

/**
 * @brief      This class describes a console.
 */
static class console
{
public:
    /**
     * @brief      This class describes an align type.
     */
    enum class align_type : uint8_t
    {
        left,
        right,
        center
    };

private:
    inline static std::recursive_mutex _mutex;
    inline static uint16_t             _comment_line = 0;
    inline static uint16_t             _y            = 1;
    inline static uint16_t             _width, _height;
    inline static bool                 _tty;

public:
    /**
     * @brief      Constructs a new instance.
     */
    console();
    /**
     * @brief      Destroys the object.
     */
    ~console();

public:
    /**
     * @brief      reference : https://github.com/jupyter-xeus/cpp-terminal
     *
     * @return     True if the specified fd is a tty, False otherwise.
     */
    static bool is_tty();

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  y     { parameter_description }
     */
    static void position(uint16_t y);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    static uint16_t position();

    /**
     * @brief      { function_description }
     */
    static void newline();

    /**
     * @brief      Clears the given line.
     *
     * @param[in]  line  The line
     */
    static void clear();

    /**
     * @brief      { function_description }
     *
     * @param[in]  align  The align
     * @param[in]  fmt    The format
     * @param      args   The arguments
     *
     * @tparam     Args   { description }
     */
    template <class... Args>
    static void put(align_type align, const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);

        auto text = std::vformat(fmt, std::make_format_args(args...));
        auto x    = 0;
        switch (align)
        {
        case align_type::right:
            x = _width - text.size() - 1;
            break;

        case align_type::center:
            x = (_width - text.size() - 1) / 2;
            break;

        default:
            x = 0;
            break;
        }

        text = std::string(x, ' ') + text;
        if (!_tty)
        {
            std::cout << text << std::endl;
            return;
        }

        auto current_line = position();
        position(current_line - _comment_line);
        clear();

        std::cout << text;
        position(current_line);

        if (!_tty)
        {
            newline();
        }
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  fmt   The format
     * @param      args  The arguments
     *
     * @tparam     Args  { description }
     */
    template <class... Args>
    static void put(const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);
        put(align_type::left, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  align  The align
     * @param[in]  fmt    The format
     * @param      args   The arguments
     *
     * @tparam     Args   { description }
     */
    template <class... Args>
    static void puts(align_type align, const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);
        if (!_tty)
        {
            put(align, fmt, std::forward<Args>(args)...);
            return;
        }
        else
        {
            put(align, fmt, std::forward<Args>(args)...);
            newline();
        }
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  fmt   The format
     * @param      args  The arguments
     *
     * @tparam     Args  { description }
     */
    template <class... Args>
    static void puts(const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);
        puts(align_type::left, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  fmt   The format
     * @param      args  The arguments
     *
     * @tparam     Args  { description }
     */
    template <class... Args>
    static void comment(const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);
        auto text = std::vformat(fmt, std::make_format_args(args...));
        if (!_tty)
        {
            std::cout << text << std::endl;
            return;
        }
        else
        {
            std::cout << std::endl << text;
            _comment_line++;
            position(_y + 1);
        }
    }
} __console;

} // namespace fb

#endif // !__CONSOLE_H__