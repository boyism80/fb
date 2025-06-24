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

private:
    inline static std::recursive_mutex _mutex;
    inline static uint16_t             _comment_line = 0;
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
     * @brief      Prints formatted text with specified alignment.
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
        auto _ = std::lock_guard(_mutex);

        auto text    = std::vformat(fmt, std::make_format_args(args...));
        auto padding = 0;
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
        if (!_tty)
        {
            std::cout << text << std::endl;
            return;
        }

        save_point();
        clear();
        std::cout << "\r" << text;
        restore_point();
    }

    /**
     * @brief      Prints formatted text with left alignment.
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
        put(align_type::left, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief      Prints formatted text with specified alignment and moves to new line.
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
        auto _ = std::lock_guard(_mutex);
        if (!_tty)
        {
            put(align, fmt, std::forward<Args>(args)...);
        }
        else
        {
            put(align, fmt, std::forward<Args>(args)...);
            newline();
        }
    }

    /**
     * @brief      Prints formatted text with left alignment and moves to new line.
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
        puts(align_type::left, fmt, std::forward<Args>(args)...);
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
        auto _    = std::lock_guard(_mutex);
        auto text = std::vformat(fmt, std::make_format_args(args...));
        if (!_tty)
        {
            std::cout << text << std::endl;
        }
        else
        {
            save_point();
            down(++_comment_line);
            std::cout << "\r" << text;
            restore_point();
        }
    }
} __console;

} // namespace fb

#endif // !__CONSOLE_H__