#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifdef _WIN32
#include <Windows.h>
#else
#include <locale.h>
#include <sys/ioctl.h>
#endif
#include <stdarg.h>
#include <sstream>
#include <cstring>
#include <mutex>
#include <format>

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
static class console
{
public:
    enum class align_type : uint8_t
    {
        left,
        right,
        center
    };

private:
    inline static std::recursive_mutex _mutex;
    inline static uint16_t             _comment_line;
    inline static uint16_t             _width, _height;

public:
    /**
     * @brief      Constructs a new instance.
     */
    console();
    /**
     * @brief      Destroys the object.
     */
    ~console();

private:
    /**
     * @brief      { function_description }
     *
     * @param[in]  text  The text
     * @param[in]  x     { parameter_description }
     * @param[in]  y     { parameter_description }
     */
    static void raw_put(const std::string& text, uint16_t x, uint16_t y);

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  x     { parameter_description }
     * @param[in]  y     { parameter_description }
     */
    static void position(uint16_t x, uint16_t y);

    /**
     * @brief      { function_description }
     *
     * @param      x     { parameter_description }
     * @param      y     { parameter_description }
     */
    static void position(uint16_t* x, uint16_t* y);

    /**
     * @brief      { function_description }
     */
    static void newline();

    /**
     * @brief      Clears the given line.
     *
     * @param[in]  line  The line
     */
    static void clear(uint16_t line = -1);

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
        auto text = std::vformat(fmt, std::make_format_args(args...));

        uint16_t x, y;
        position(nullptr, &y);

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
        position(0, y);
        clear();
        raw_put(text, x, y);
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
        put(align_type::left, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
    static void puts(align_type align, const std::string& fmt, Args&&... args)
    {
        put(align, fmt, std::forward<Args>(args)...);
        newline();
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
        uint16_t x, y;
        position(&x, &y);

        position(0, y + (++_comment_line));
        put(fmt, std::forward<Args>(args)...);

        position(x, y);
    }
} __console;

} // namespace fb

#endif // !__CONSOLE_H__