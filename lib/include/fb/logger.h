#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <fb/console.h>
#include <fb/config.h>
#include <boost/algorithm/string.hpp>

namespace fb {

class logger
{
public:
    enum level
    {
        NONE  = 0x0000,
        DEBUG = 0x0001,
        INFO  = 0x0002,
        WARN  = 0x0004,
        FATAL = 0x0008
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
    template <class... Args>
    static fb::logger& debug(const std::string& fmt, Args&&... args);
    template <class... Args>
    static fb::logger& info(const std::string& fmt, Args&&... args);
    template <class... Args>
    static fb::logger& warn(const std::string& fmt, Args&&... args);
    template <class... Args>
    static fb::logger& fatal(const std::string& fmt, Args&&... args);
};

} // namespace fb

template <class... Args>
fb::logger& fb::logger::debug(const std::string& fmt, Args&&... args)
{
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::DEBUG) == false)
        return ist;

    auto  message = std::vformat(fmt, std::make_format_args(args...));
    auto& c       = fb::console::get();
    c.puts("[DEBUG] {}", message);

    return ist;
}

template <class... Args>
fb::logger& fb::logger::info(const std::string& fmt, Args&&... args)
{
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::INFO) == false)
        return ist;

    auto  message = std::vformat(fmt, std::make_format_args(args...));
    auto& c       = fb::console::get();
    c.puts("[INFO] {}", message);

    return ist;
}

template <class... Args>
fb::logger& fb::logger::warn(const std::string& fmt, Args&&... args)
{
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::WARN) == false)
        return ist;

    auto  message = std::vformat(fmt, std::make_format_args(args...));
    auto& c       = fb::console::get();
    c.puts("[WARN] {}", message);

    return ist;
}

template <class... Args>
fb::logger& fb::logger::fatal(const std::string& fmt, Args&&... args)
{
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::FATAL) == false)
        return ist;

    auto  message = std::vformat(fmt, std::make_format_args(args...));
    auto& c       = fb::console::get();
    c.puts("[FATAL] {}", message);

    return ist;
}

#endif // !__LOGGER_H__