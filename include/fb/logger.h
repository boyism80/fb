#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <fb/config.h>
#include <fb/model/datetime.h>
#include <boost/algorithm/string.hpp>

namespace fb {

class logger
{
public:
    enum level
    {
        NONE  = 0x0000, ///< No logging
        DEBUG = 0x0001, ///< Debug level messages
        INFO  = 0x0002, ///< Informational messages
        WARN  = 0x0004, ///< Warning messages
        FATAL = 0x0008  ///< Fatal error messages
    };

private:
    level _level;

private:
    logger(fb::logger::level level);

public:
    logger(const logger&) = delete;
    logger(logger&&)      = delete;
    ~logger()             = default;

public:
    logger& operator= (logger&)       = delete;
    logger& operator= (const logger&) = delete;

private:
    bool           has_flag(fb::logger::level level) const;
    static logger& get();

public:
    template <class... Args> static fb::logger& debug(const std::string& fmt, Args&&... args);
    template <class... Args> static fb::logger& debug(fb::console::color color, const std::string& fmt, Args&&... args);
    template <class... Args> static fb::logger& info(const std::string& fmt, Args&&... args);
    template <class... Args> static fb::logger& info(fb::console::color color, const std::string& fmt, Args&&... args);
    template <class... Args> static fb::logger& warn(const std::string& fmt, Args&&... args);
    template <class... Args> static fb::logger& warn(fb::console::color color, const std::string& fmt, Args&&... args);
    template <class... Args> static fb::logger& fatal(const std::string& fmt, Args&&... args);
    template <class... Args> static fb::logger& fatal(fb::console::color color, const std::string& fmt, Args&&... args);
    template <class... Args> static fb::logger& comment(fb::console::color color, const std::string& fmt, Args&&... args);
    template <class... Args> static fb::logger& comment(const std::string& fmt, Args&&... args);
};

} // namespace fb

template <class... Args>
fb::logger& fb::logger::debug(const std::string& fmt, Args&&... args)
{
    return debug(fb::console::color::light_gray, fmt, std::forward<Args>(args)...);
}

template <class... Args>
fb::logger& fb::logger::debug(fb::console::color color, const std::string& fmt, Args&&... args)
{
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::DEBUG) == false)
        return ist;

    auto message = std::vformat(fmt, std::make_format_args(args...));
    fb::console::puts(color, "{:<7} {} {}", "[DEBUG]", fb::model::datetime().to_string(), message);

    return ist;
}

template <class... Args>
fb::logger& fb::logger::info(const std::string& fmt, Args&&... args)
{
    return info(fb::console::color::white, fmt, std::forward<Args>(args)...);
}

template <class... Args>
fb::logger& fb::logger::info(fb::console::color color, const std::string& fmt, Args&&... args)
{
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::INFO) == false)
        return ist;

    auto message = std::vformat(fmt, std::make_format_args(args...));
    fb::console::puts(color, "{:<7} {} {}", "[INFO]", fb::model::datetime().to_string(), message);

    return ist;
}

template <class... Args>
fb::logger& fb::logger::warn(const std::string& fmt, Args&&... args)
{
    return warn(fb::console::color::yellow, fmt, std::forward<Args>(args)...);
}

template <class... Args>
fb::logger& fb::logger::warn(fb::console::color color, const std::string& fmt, Args&&... args)
{
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::WARN) == false)
        return ist;

    auto message = std::vformat(fmt, std::make_format_args(args...));
    fb::console::puts(color, "{:<7} {} {}", "[WARN]", fb::model::datetime().to_string(), message);

    return ist;
}

template <class... Args>
fb::logger& fb::logger::fatal(const std::string& fmt, Args&&... args)
{
    return fatal(fb::console::color::red, fmt, std::forward<Args>(args)...);
}

template <class... Args>
fb::logger& fb::logger::fatal(fb::console::color color, const std::string& fmt, Args&&... args)
{
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::FATAL) == false)
        return ist;

    auto message = std::vformat(fmt, std::make_format_args(args...));
    fb::console::puts(color, "{:<7} {} {}", "[FATAL]", fb::model::datetime().to_string(), message);

    return ist;
}

template <class... Args>
fb::logger& fb::logger::comment(fb::console::color color, const std::string& fmt, Args&&... args)
{
    auto& ist     = get();
    auto  message = std::vformat(fmt, std::make_format_args(args...));
    fb::console::comment(color, message);

    return ist;
}

template <class... Args>
fb::logger& fb::logger::comment(const std::string& fmt, Args&&... args)
{
    return comment(fb::console::color::white, fmt, std::forward<Args>(args)...);
}

#endif // !__LOGGER_H__