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
#include <cmath>
#include <vector>
#include <unordered_map>
#include <memory>

#ifdef _WIN32
#define CONSOLE_TITLE "Private kingdom of the wind - http://cshyeon.com"
bool set_console_icon(int id);
#endif

namespace fb {

static class console
{
public:
    enum class align_type : uint8_t
    {
        left,
        right,
        center
    };

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

    enum class mode : uint8_t
    {
        tty,
        plain
    };

private:
    // Inheritance-based policy pattern
    class console_policy
    {
    protected:
        console_policy() = default;

    public:
        virtual ~console_policy()                                                 = default;
        virtual void put(align_type align, color color, const std::string& text)  = 0;
        virtual void puts(align_type align, color color, const std::string& text) = 0;
        virtual void comment(color color, const std::string& text)                = 0;
        virtual void newline()                                                    = 0;
        virtual void clear()                                                      = 0;
        virtual void save_point()                                                 = 0;
        virtual void restore_point()                                              = 0;
        virtual void up(uint8_t line)                                             = 0;
        virtual void down(uint8_t line)                                           = 0;
        virtual void progress(const std::string& text, float progress)            = 0;

    protected:
        // Common alignment logic used by both policies
        std::string get_aligned_text(align_type align, const std::string& text);
    };

    class tty_policy : public console_policy
    {
    private:
        uint16_t _comment_line = 0;

    public:
        void put(align_type align, color color, const std::string& text) override;
        void puts(align_type align, color color, const std::string& text) override;
        void comment(color color, const std::string& text) override;
        void newline() override;
        void clear() override;
        void save_point() override;
        void restore_point() override;
        void up(uint8_t line) override;
        void down(uint8_t line) override;
        void progress(const std::string& text, float progress) override;
    };

    class plain_policy : public console_policy
    {
    public:
        void put(align_type align, color color, const std::string& text) override;
        void puts(align_type align, color color, const std::string& text) override;
        void comment(color color, const std::string& text) override;
        void newline() override;
        void clear() override;
        void save_point() override;
        void restore_point() override;
        void up(uint8_t line) override;
        void down(uint8_t line) override;
        void progress(const std::string& text, float progress) override;
    };

private:
    using policy_map = std::unordered_map<mode, std::unique_ptr<console_policy>>;
    using color_map  = std::unordered_map<color, std::string>;

private:
    inline static std::recursive_mutex _mutex;
    inline static uint16_t             _width, _height;
    inline static bool                 _system_tty;       ///< System TTY capability
    inline static mode                 _mode = mode::tty; ///< User-configured output mode
    inline static policy_map           _policies;         ///< Policy instances by mode
    inline static console_policy*      _current_policy;   ///< Current active policy
    inline static color_map            _color_codes = {
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
    console();
    ~console();

public:
    static bool        is_tty();
    static mode        get_mode();
    static void        set_mode(mode new_mode);
    static void        update_policy();
    static std::string colorize(color color);

public:
    static void newline();
    static void clear();
    static void save_point();
    static void restore_point();
    static void up(uint8_t line);
    static void down(uint8_t line);
    static void progress(const std::string& text, float progress);

public:
    template <class... Args>
    static void put(align_type align, color color, const std::string& fmt, Args&&... args)
    {
        auto _    = std::lock_guard(_mutex);
        auto text = std::vformat(fmt, std::make_format_args(args...));
        _current_policy->put(align, color, text);
    }
    template <class... Args>
    static void put(align_type align, const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);
        put(align, color::reset, fmt, std::forward<Args>(args)...);
    }
    template <class... Args>
    static void put(const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);
        put(align_type::left, color::reset, fmt, std::forward<Args>(args)...);
    }
    template <class... Args>
    static void puts(align_type align, color color, const std::string& fmt, Args&&... args)
    {
        auto _    = std::lock_guard(_mutex);
        auto text = std::vformat(fmt, std::make_format_args(args...));
        _current_policy->puts(align, color, text);
    }
    template <class... Args>
    static void puts(color color, const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);
        puts(align_type::left, color, fmt, std::forward<Args>(args)...);
    }
    template <class... Args>
    static void puts(align_type align, const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);
        puts(align, color::reset, fmt, std::forward<Args>(args)...);
    }
    template <class... Args>
    static void puts(const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);
        puts(align_type::left, color::reset, fmt, std::forward<Args>(args)...);
    }
    template <class... Args>
    static void comment(color color, const std::string& fmt, Args&&... args)
    {
        auto _    = std::lock_guard(_mutex);
        auto text = std::vformat(fmt, std::make_format_args(args...));
        _current_policy->comment(color, text);
    }
    template <class... Args>
    static void comment(const std::string& fmt, Args&&... args)
    {
        auto _ = std::lock_guard(_mutex);
        comment(color::reset, fmt, std::forward<Args>(args)...);
    }
} __console;

} // namespace fb

#endif // !__CONSOLE_H__