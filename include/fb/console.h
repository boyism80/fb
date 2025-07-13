#ifndef __CONSOLE_H__
#define __CONSOLE_H__

/**
 * @file    console.h
 * @brief   Cross-platform console utility for formatted output and terminal control
 * @author  FB Development Team
 *
 * @details This file implements a comprehensive cross-platform console utility system
 *          that provides formatted text output, progress bars, cursor control, and
 *          terminal detection for the FB 2D MMORPG server. The system handles platform
 *          differences between Windows and Unix-like systems transparently.
 *
 *          Key features:
 *          - Cross-platform console output with Windows and Unix support
 *          - TTY detection for appropriate output formatting in different environments
 *          - Formatted text output with alignment options (left, right, center)
 *          - Progress bar display with percentage and text indicators
 *          - Cursor positioning and movement control for dynamic output
 *          - Thread-safe operations using recursive mutex for multi-threaded access
 *          - Template-based formatted output with std::format integration
 *          - Automatic fallback for non-TTY environments (pipes, redirects)
 *          - Terminal size detection and management for proper formatting
 *          - Comment system for temporary status messages and debugging
 *
 * @note    The console system provides the primary user interface for server
 *          administration and monitoring, supporting both interactive terminal
 *          usage and automated deployment scenarios with appropriate output formatting.
 */

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
#include <cmath>
#include <vector>
#include <unordered_map>

#ifdef _WIN32
#define CONSOLE_TITLE "Private kingdom of the wind - http://cshyeon.com"
bool set_console_icon(int id);
#endif

namespace fb {

/**
 * @brief      Cross-platform console utility class for formatted output and terminal control.
 *
 *             This static class provides comprehensive console functionality including
 *             formatted text output with alignment options, progress bar display,
 *             cursor positioning, and terminal detection. It supports both TTY and
 *             non-TTY environments with appropriate fallbacks, and includes thread-safe
 *             operations for multi-threaded applications. The class handles platform
 *             differences between Windows and Unix-like systems transparently.
 */
static class console
{
public:
    /**
     * @brief      Text alignment options for console output formatting.
     */
    enum class align_type : uint8_t
    {
        left,
        right,
        center
    };

    /**
     * @brief      Console text color options.
     */
    enum class color : uint8_t
    {
        reset,
        black,
        red,
        green,
        yellow,
        blue,
        magenta,
        cyan,
        white,
        light_gray,
        light_red,
        light_green,
        light_yellow,
        light_blue,
        light_magenta,
        light_cyan,
        bright_white
    };

    /**
     * @brief      Console output mode options.
     *
     *             Controls how console output is formatted and displayed.
     *             The actual behavior depends on both the mode setting and
     *             system TTY availability.
     */
    enum class mode : uint8_t
    {
        tty,  ///< Use TTY features (colors, progress bars, cursor control)
        plain ///< Use plain text output only
    };

private:
    inline static std::recursive_mutex                   _mutex;
    inline static uint16_t                               _comment_line = 0;
    inline static uint16_t                               _width, _height;
    inline static bool                                   _system_tty;              ///< System TTY capability
    inline static mode                                   _mode        = mode::tty; ///< User-configured output mode
    inline static std::unordered_map<color, std::string> _color_codes = {
        {color::reset,         "\033[0m" },
        {color::black,         "\033[30m"},
        {color::red,           "\033[31m"},
        {color::green,         "\033[32m"},
        {color::yellow,        "\033[33m"},
        {color::blue,          "\033[34m"},
        {color::magenta,       "\033[35m"},
        {color::cyan,          "\033[36m"},
        {color::white,         "\033[37m"},
        {color::light_gray,    "\033[90m"},
        {color::light_red,     "\033[91m"},
        {color::light_green,   "\033[92m"},
        {color::light_yellow,  "\033[93m"},
        {color::light_blue,    "\033[94m"},
        {color::light_magenta, "\033[95m"},
        {color::light_cyan,    "\033[96m"},
        {color::bright_white,  "\033[97m"}
    };

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
     * @return     True if the system supports TTY, False otherwise.
     */
    static bool is_tty();

    /**
     * @brief      Gets the current console output mode.
     *
     * @return     The current mode setting.
     */
    static mode get_mode();

    /**
     * @brief      Sets the console output mode.
     *
     * @param[in]  new_mode  The new mode to set.
     *
     * @note       Setting TTY mode when system TTY is not available
     *             will still result in plain text output.
     */
    static void set_mode(mode new_mode);

    /**
     * @brief      Determines if TTY features should be used.
     *
     *             This considers both system TTY capability and user mode setting.
     *
     * @return     True if TTY features should be used, False for plain text.
     */
    static bool is_effective_tty();

    /**
     * @brief      Converts a color enum to ANSI color code string.
     *
     * @param[in]  color  The color to convert.
     *
     * @return     ANSI color code string.
     */
    static std::string colorize(color color);

public:
    /**
     * @brief      Moves the cursor to a new line.
     */
    static void newline();

    /**
     * @brief      Clears the current line.
     */
    static void clear();

    /**
     * @brief      Saves a point.
     */
    static void save_point();

    /**
     * @brief      Restores the previously saved cursor position.
     */
    static void restore_point();

    /**
     * @brief      Moves the cursor up by the specified number of lines.
     *
     * @param[in]  line  The number of lines to move up.
     */
    static void up(uint8_t line);

    /**
     * @brief      Moves the cursor down by the specified number of lines.
     *
     * @param[in]  line  The number of lines to move down.
     */
    static void down(uint8_t line);

    /**
     * @brief      Displays a progress bar with text and percentage.
     *
     * @param[in]  text      The text to display alongside the progress bar.
     * @param[in]  progress  The progress percentage (0.0 to 100.0).
     */
    static void progress(const std::string& text, float progress);

    /**
     * @brief      Prints formatted text with specified alignment, color, and formatting.
     *
     * @param[in]  align  The text alignment (left, right, center).
     * @param[in]  color  The text color.
     * @param[in]  fmt    The format string.
     * @param      args   The arguments for formatting.
     *
     * @tparam     Args   The types of the formatting arguments.
     */
    template <class... Args>
    static void put(align_type align, color color, const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);

        auto text    = std::vformat(fmt, std::make_format_args(args...));
        auto padding = uint16_t{0};
        switch (align)
        {
        case align_type::right:
            padding = _width - text.size() - 1;
            break;

        case align_type::center:
            padding = (_width - text.size() - 1) / 2;
            break;

        default:
            padding = 0;
            break;
        }

        text = std::string(padding, ' ') + text;
        if (!is_effective_tty())
        {
            std::cout << colorize(color) << text << colorize(color::reset) << std::endl;
            return;
        }

        save_point();
        clear();
        std::cout << "\r" << colorize(color) << text << colorize(color::reset);
        restore_point();
    }

    /**
     * @brief      Prints formatted text with specified alignment and formatting.
     *
     * @param[in]  align  The text alignment (left, right, center).
     * @param[in]  fmt    The format string.
     * @param      args   The arguments for formatting.
     *
     * @tparam     Args   The types of the formatting arguments.
     */
    template <class... Args>
    static void put(align_type align, const std::string& fmt, Args&&... args)
    {
        put(align, color::reset, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief      Prints formatted text with left alignment and formatting.
     *
     * @param[in]  fmt   The format string.
     * @param      args  The arguments for formatting.
     *
     * @tparam     Args  The types of the formatting arguments.
     */
    template <class... Args>
    static void put(const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);
        put(align_type::left, color::reset, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief      Prints formatted text with specified alignment, color, and formatting, then moves to new line.
     *
     * @param[in]  align  The text alignment (left, right, center).
     * @param[in]  color  The text color.
     * @param[in]  fmt    The format string.
     * @param      args   The arguments for formatting.
     *
     * @tparam     Args   The types of the formatting arguments.
     */
    template <class... Args>
    static void puts(align_type align, color color, const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);
        put(align, color, fmt, std::forward<Args>(args)...);
        if (is_effective_tty())
        {
            newline();
        }
    }

    /**
     * @brief      Prints formatted text with left alignment and formatting, then moves to new line.
     *
     * @param[in]  color  The text color.
     * @param[in]  fmt    The format string.
     * @param      args   The arguments for formatting.
     *
     * @tparam     Args   The types of the formatting arguments.
     */
    template <class... Args>
    static void puts(color color, const std::string& fmt, Args&&... args)
    {
        puts(align_type::left, color, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief      Prints formatted text with specified alignment and formatting, then moves to new line.
     *
     * @param[in]  align  The text alignment (left, right, center).
     * @param[in]  fmt    The format string.
     * @param      args   The arguments for formatting.
     *
     * @tparam     Args   The types of the formatting arguments.
     */
    template <class... Args>
    static void puts(align_type align, const std::string& fmt, Args&&... args)
    {
        puts(align, color::reset, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief      Prints formatted text with left alignment and formatting, then moves to new line.
     *
     * @param[in]  fmt   The format string.
     * @param      args  The arguments for formatting.
     *
     * @tparam     Args  The types of the formatting arguments.
     */
    template <class... Args>
    static void puts(const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);
        puts(align_type::left, color::reset, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief      Prints a comment below the current line without affecting cursor position.
     *
     * @param[in]  color  The text color.
     * @param[in]  fmt    The format string.
     * @param      args   The arguments for formatting.
     *
     * @tparam     Args   The types of the formatting arguments.
     */
    template <class... Args>
    static void comment(color color, const std::string& fmt, Args&&... args)
    {
        auto _    = std::lock_guard(_mutex);
        auto text = std::vformat(fmt, std::make_format_args(args...));
        if (!is_effective_tty())
        {
            std::cout << colorize(color) << text << colorize(color::reset) << std::endl;
        }
        else
        {
            save_point();
            down(++_comment_line);
            std::cout << "\r" << colorize(color) << text << colorize(color::reset);
            restore_point();
        }
    }

    /**
     * @brief      Prints a comment below the current line without affecting cursor position.
     *
     * @param[in]  fmt   The format string.
     * @param      args  The arguments for formatting.
     *
     * @tparam     Args  The types of the formatting arguments.
     */
    template <class... Args>
    static void comment(const std::string& fmt, Args&&... args)
    {
        comment(color::reset, fmt, std::forward<Args>(args)...);
    }
} __console;

} // namespace fb

#endif // !__CONSOLE_H__