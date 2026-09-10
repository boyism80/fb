#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <fb/config.h>
#include <fb/encoding.h>
#include <fb/model/datetime.h>
#include <boost/algorithm/string.hpp>
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <deque>
#include <format>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

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
    level                   _level;
    std::string             _service;
    std::string             _name;
    std::deque<std::string> _queue;
    std::mutex              _queue_mutex;
    std::condition_variable _queue_cv;
    std::atomic<bool>       _stop{false};
    std::thread             _writer;
    static constexpr size_t _queue_limit = 4096;

private:
    logger(fb::logger::level level, std::string service, std::string name);

public:
    logger(const logger&) = delete;
    logger(logger&&)      = delete;
    ~logger();

public:
    logger& operator= (logger&)       = delete;
    logger& operator= (const logger&) = delete;

private:
    bool               has_flag(fb::logger::level level) const;
    void               enqueue(std::string line);
    void               writer_run();
    static logger&     get();
    static std::string format_line(std::string_view tag, std::string_view name, std::string_view message);

public:
    static std::string daily_path(std::string_view service);

public:
    template <class... Args> static fb::logger& debug(std::string_view fmt, Args&&... args);
    template <class... Args> static fb::logger& debug(fb::console::color color, std::string_view fmt, Args&&... args);
    template <class... Args> static fb::logger& info(std::string_view fmt, Args&&... args);
    template <class... Args> static fb::logger& info(fb::console::color color, std::string_view fmt, Args&&... args);
    template <class... Args> static fb::logger& warn(std::string_view fmt, Args&&... args);
    template <class... Args> static fb::logger& warn(fb::console::color color, std::string_view fmt, Args&&... args);
    template <class... Args> static fb::logger& fatal(std::string_view fmt, Args&&... args);
    template <class... Args> static fb::logger& fatal(fb::console::color color, std::string_view fmt, Args&&... args);
    template <class... Args> static fb::logger& comment(fb::console::color color, std::string_view fmt, Args&&... args);
    template <class... Args> static fb::logger& comment(std::string_view fmt, Args&&... args);
};

} // namespace fb

template <class... Args>
fb::logger& fb::logger::debug(std::string_view fmt, Args&&... args)
{
    return debug(fb::console::color::light_gray, fmt, std::forward<Args>(args)...);
}

template <class... Args>
fb::logger& fb::logger::debug(fb::console::color color, std::string_view fmt, Args&&... args)
{
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::DEBUG) == false)
        return ist;

    auto message = std::vformat(fmt, std::make_format_args(args...));
    auto line    = format_line("[DEBUG]", ist._name, message);
#ifdef _WIN32
    line = fb::utf8(line);
#endif
    fb::console::puts(color, "{}", line);
    ist.enqueue(std::move(line));

    return ist;
}

template <class... Args>
fb::logger& fb::logger::info(std::string_view fmt, Args&&... args)
{
    return info(fb::console::color::white, fmt, std::forward<Args>(args)...);
}

template <class... Args>
fb::logger& fb::logger::info(fb::console::color color, std::string_view fmt, Args&&... args)
{
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::INFO) == false)
        return ist;

    auto message = std::vformat(fmt, std::make_format_args(args...));
    auto line    = format_line("[INFO]", ist._name, message);
#ifdef _WIN32
    line = fb::utf8(line);
#endif
    fb::console::puts(color, "{}", line);
    ist.enqueue(std::move(line));

    return ist;
}

template <class... Args>
fb::logger& fb::logger::warn(std::string_view fmt, Args&&... args)
{
    return warn(fb::console::color::yellow, fmt, std::forward<Args>(args)...);
}

template <class... Args>
fb::logger& fb::logger::warn(fb::console::color color, std::string_view fmt, Args&&... args)
{
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::WARN) == false)
        return ist;

    auto message = std::vformat(fmt, std::make_format_args(args...));
    auto line    = format_line("[WARN]", ist._name, message);
#ifdef _WIN32
    line = fb::utf8(line);
#endif
    fb::console::puts(color, "{}", line);
    ist.enqueue(std::move(line));

    return ist;
}

template <class... Args>
fb::logger& fb::logger::fatal(std::string_view fmt, Args&&... args)
{
    return fatal(fb::console::color::red, fmt, std::forward<Args>(args)...);
}

template <class... Args>
fb::logger& fb::logger::fatal(fb::console::color color, std::string_view fmt, Args&&... args)
{
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::FATAL) == false)
        return ist;

    auto message = std::vformat(fmt, std::make_format_args(args...));
    auto line    = format_line("[FATAL]", ist._name, message);
#ifdef _WIN32
    line = fb::utf8(line);
#endif
    fb::console::puts(color, "{}", line);
    std::fprintf(stderr, "%s\n", line.c_str());
    std::fflush(stderr);
    ist.enqueue(std::move(line));

    return ist;
}

template <class... Args>
fb::logger& fb::logger::comment(fb::console::color color, std::string_view fmt, Args&&... args)
{
    auto& ist     = get();
    auto  message = std::vformat(fmt, std::make_format_args(args...));
    fb::console::comment(color, message);

    return ist;
}

template <class... Args>
fb::logger& fb::logger::comment(std::string_view fmt, Args&&... args)
{
    return comment(fb::console::color::white, fmt, std::forward<Args>(args)...);
}

#endif // !__LOGGER_H__
