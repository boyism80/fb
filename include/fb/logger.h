#ifndef __LOGGER_H__
#define __LOGGER_H__

/**
 * @file    logger.h
 * @brief   Thread-safe singleton logging system with configurable level filtering
 * @author  FB Development Team
 *
 * @details This file implements a comprehensive logging system that provides thread-safe,
 *          level-based logging functionality for the FB 2D MMORPG server. The system
 *          supports multiple log levels with configurable filtering and formatted
 *          output through the console system with singleton pattern for global access.
 *
 *          Key features:
 *          - Thread-safe singleton logger with global access throughout the application
 *          - Multiple log levels (DEBUG, INFO, WARN, FATAL) with bitwise combination support
 *          - Configurable log level filtering through the configuration system
 *          - Template-based formatted logging with std::format integration
 *          - Automatic log level prefix injection for easy identification
 *          - Integration with the console system for consistent output formatting
 *          - Method chaining support for fluent logging interface
 *          - Configuration-driven log level control for production/debug builds
 *          - Exception-safe logging operations with proper error handling
 *          - Performance optimization with early level checking to avoid formatting overhead
 *
 * @note    The logging system is initialized automatically and provides the primary
 *          debugging and monitoring infrastructure for the FB 2D MMORPG server,
 *          enabling comprehensive diagnostics and troubleshooting capabilities.
 */

#include <fb/config.h>
#include <fb/model/datetime.h>
#include <boost/algorithm/string.hpp>

namespace fb {

/**
 * @brief      A singleton logger class that provides level-based logging functionality.
 *
 *             This class implements a thread-safe singleton logger that supports
 *             multiple log levels (DEBUG, INFO, WARN, FATAL) with configurable
 *             filtering. Log levels are configured through the configuration system
 *             and messages are output through the console system.
 */
class logger
{
public:
    /**
     * @brief      Enumeration of available log levels.
     *
     *             Log levels can be combined using bitwise OR operations to
     *             enable multiple levels simultaneously.
     */
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
    /**
     * @brief      Private constructor for singleton pattern.
     *
     * @param[in]  level  The log level configuration.
     */
    logger(fb::logger::level level);

public:
    /**
     * @brief      Copy constructor is deleted to maintain singleton pattern.
     */
    logger(const logger&) = delete;

    /**
     * @brief      Move constructor is deleted to maintain singleton pattern.
     */
    logger(logger&&) = delete;

    /**
     * @brief      Destructor is defaulted.
     */
    ~logger() = default;

public:
    /**
     * @brief      Assignment operator is deleted to maintain singleton pattern.
     */
    logger& operator= (logger&) = delete;

    /**
     * @brief      Const assignment operator is deleted to maintain singleton pattern.
     */
    logger& operator= (const logger&) = delete;

private:
    /**
     * @brief      Checks if the specified log level is enabled.
     *
     * @param[in]  level  The log level to check.
     *
     * @return     True if the log level is enabled, false otherwise.
     */
    bool has_flag(fb::logger::level level) const;

    /**
     * @brief      Gets the singleton logger instance.
     *
     *             Creates the logger instance on first call using configuration
     *             from the "log" section. Thread-safe initialization using std::once_flag.
     *
     * @return     Reference to the singleton logger instance.
     */
    static logger& get();

public:
    /**
     * @brief      Logs a debug message with formatted arguments.
     *
     *             Only outputs the message if DEBUG level is enabled in configuration.
     *             Uses std::format for string formatting and outputs with [DEBUG] prefix.
     *
     * @param[in]  fmt   The format string.
     * @param[in]  args  Arguments for string formatting.
     *
     * @tparam     Args  Parameter pack for format arguments.
     *
     * @return     Reference to the logger instance for method chaining.
     */
    template <class... Args>
    static fb::logger& debug(const std::string& fmt, Args&&... args);

    /**
     * @brief      Logs a debug message with color and formatted arguments.
     *
     *             Only outputs the message if DEBUG level is enabled in configuration.
     *             Uses std::format for string formatting and outputs with [DEBUG] prefix and specified color.
     *
     * @param[in]  color  The text color.
     * @param[in]  fmt    The format string.
     * @param[in]  args   Arguments for string formatting.
     *
     * @tparam     Args   Parameter pack for format arguments.
     *
     * @return     Reference to the logger instance for method chaining.
     */
    template <class... Args>
    static fb::logger& debug(fb::console::color color, const std::string& fmt, Args&&... args);

    /**
     * @brief      Logs an info message with formatted arguments.
     *
     *             Only outputs the message if INFO level is enabled in configuration.
     *             Uses std::format for string formatting and outputs with [INFO] prefix.
     *
     * @param[in]  fmt   The format string.
     * @param[in]  args  Arguments for string formatting.
     *
     * @tparam     Args  Parameter pack for format arguments.
     *
     * @return     Reference to the logger instance for method chaining.
     */
    template <class... Args>
    static fb::logger& info(const std::string& fmt, Args&&... args);

    /**
     * @brief      Logs an info message with color and formatted arguments.
     *
     *             Only outputs the message if INFO level is enabled in configuration.
     *             Uses std::format for string formatting and outputs with [INFO] prefix and specified color.
     *
     * @param[in]  color  The text color.
     * @param[in]  fmt    The format string.
     * @param[in]  args   Arguments for string formatting.
     *
     * @tparam     Args   Parameter pack for format arguments.
     *
     * @return     Reference to the logger instance for method chaining.
     */
    template <class... Args>
    static fb::logger& info(fb::console::color color, const std::string& fmt, Args&&... args);

    /**
     * @brief      Logs a warning message with formatted arguments.
     *
     *             Only outputs the message if WARN level is enabled in configuration.
     *             Uses std::format for string formatting and outputs with [WARN] prefix.
     *
     * @param[in]  fmt   The format string.
     * @param[in]  args  Arguments for string formatting.
     *
     * @tparam     Args  Parameter pack for format arguments.
     *
     * @return     Reference to the logger instance for method chaining.
     */
    template <class... Args>
    static fb::logger& warn(const std::string& fmt, Args&&... args);

    /**
     * @brief      Logs a warning message with color and formatted arguments.
     *
     *             Only outputs the message if WARN level is enabled in configuration.
     *             Uses std::format for string formatting and outputs with [WARN] prefix and specified color.
     *
     * @param[in]  color  The text color.
     * @param[in]  fmt    The format string.
     * @param[in]  args   Arguments for string formatting.
     *
     * @tparam     Args   Parameter pack for format arguments.
     *
     * @return     Reference to the logger instance for method chaining.
     */
    template <class... Args>
    static fb::logger& warn(fb::console::color color, const std::string& fmt, Args&&... args);

    /**
     * @brief      Logs a fatal error message with formatted arguments.
     *
     *             Only outputs the message if FATAL level is enabled in configuration.
     *             Uses std::format for string formatting and outputs with [FATAL] prefix.
     *
     * @param[in]  fmt   The format string.
     * @param[in]  args  Arguments for string formatting.
     *
     * @tparam     Args  Parameter pack for format arguments.
     *
     * @return     Reference to the logger instance for method chaining.
     */
    template <class... Args>
    static fb::logger& fatal(const std::string& fmt, Args&&... args);

    /**
     * @brief      Logs a fatal error message with color and formatted arguments.
     *
     *             Only outputs the message if FATAL level is enabled in configuration.
     *             Uses std::format for string formatting and outputs with [FATAL] prefix and specified color.
     *
     * @param[in]  color  The text color.
     * @param[in]  fmt    The format string.
     * @param[in]  args   Arguments for string formatting.
     *
     * @tparam     Args   Parameter pack for format arguments.
     *
     * @return     Reference to the logger instance for method chaining.
     */
    template <class... Args>
    static fb::logger& fatal(fb::console::color color, const std::string& fmt, Args&&... args);

    /**
     * @brief      Logs a comment message with formatted arguments and optional color.
     *
     *             Outputs the message with optional color through the console system.
     *             Uses std::format for string formatting.
     *
     * @param[in]  color  The text color (optional).
     * @param[in]  fmt    The format string.
     * @param[in]  args   Arguments for string formatting.
     *
     * @tparam     Args   Parameter pack for format arguments.
     *
     * @return     Reference to the logger instance for method chaining.
     */
    template <class... Args>
    static fb::logger& comment(fb::console::color color, const std::string& fmt, Args&&... args);

    /**
     * @brief      Logs a comment message with formatted arguments.
     *
     *             Outputs the message through the console system.
     *             Uses std::format for string formatting.
     *
     * @param[in]  fmt   The format string.
     * @param[in]  args  Arguments for string formatting.
     *
     * @tparam     Args  Parameter pack for format arguments.
     *
     * @return     Reference to the logger instance for method chaining.
     */
    template <class... Args>
    static fb::logger& comment(const std::string& fmt, Args&&... args);
};

} // namespace fb

/**
 * @brief      Template implementation for debug logging.
 *
 *             Checks if DEBUG level is enabled, formats the message using std::vformat,
 *             and outputs it with [DEBUG] prefix through the console system.
 *
 * @param[in]  fmt   The format string.
 * @param[in]  args  Arguments for string formatting.
 *
 * @tparam     Args  Parameter pack for format arguments.
 *
 * @return     Reference to the logger instance for method chaining.
 */
template <class... Args>
fb::logger& fb::logger::debug(const std::string& fmt, Args&&... args)
{
    return debug(fb::console::color::light_gray, fmt, std::forward<Args>(args)...);
}

/**
 * @brief      Template implementation for debug logging with color.
 *
 *             Checks if DEBUG level is enabled, formats the message using std::vformat,
 *             and outputs it with [DEBUG] prefix and specified color through the console system.
 *
 * @param[in]  color  The text color.
 * @param[in]  fmt    The format string.
 * @param[in]  args   Arguments for string formatting.
 *
 * @tparam     Args   Parameter pack for format arguments.
 *
 * @return     Reference to the logger instance for method chaining.
 */
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

/**
 * @brief      Template implementation for info logging.
 *
 *             Checks if INFO level is enabled, formats the message using std::vformat,
 *             and outputs it with [INFO] prefix through the console system.
 *
 * @param[in]  fmt   The format string.
 * @param[in]  args  Arguments for string formatting.
 *
 * @tparam     Args  Parameter pack for format arguments.
 *
 * @return     Reference to the logger instance for method chaining.
 */
template <class... Args>
fb::logger& fb::logger::info(const std::string& fmt, Args&&... args)
{
    return info(fb::console::color::white, fmt, std::forward<Args>(args)...);
}

/**
 * @brief      Template implementation for info logging with color.
 *
 *             Checks if INFO level is enabled, formats the message using std::vformat,
 *             and outputs it with [INFO] prefix and specified color through the console system.
 *
 * @param[in]  color  The text color.
 * @param[in]  fmt    The format string.
 * @param[in]  args   Arguments for string formatting.
 *
 * @tparam     Args   Parameter pack for format arguments.
 *
 * @return     Reference to the logger instance for method chaining.
 */
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

/**
 * @brief      Template implementation for warning logging.
 *
 *             Checks if WARN level is enabled, formats the message using std::vformat,
 *             and outputs it with [WARN] prefix through the console system.
 *
 * @param[in]  fmt   The format string.
 * @param[in]  args  Arguments for string formatting.
 *
 * @tparam     Args  Parameter pack for format arguments.
 *
 * @return     Reference to the logger instance for method chaining.
 */
template <class... Args>
fb::logger& fb::logger::warn(const std::string& fmt, Args&&... args)
{
    return warn(fb::console::color::yellow, fmt, std::forward<Args>(args)...);
}

/**
 * @brief      Template implementation for warning logging with color.
 *
 *             Checks if WARN level is enabled, formats the message using std::vformat,
 *             and outputs it with [WARN] prefix and specified color through the console system.
 *
 * @param[in]  color  The text color.
 * @param[in]  fmt    The format string.
 * @param[in]  args   Arguments for string formatting.
 *
 * @tparam     Args   Parameter pack for format arguments.
 *
 * @return     Reference to the logger instance for method chaining.
 */
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

/**
 * @brief      Template implementation for fatal error logging.
 *
 *             Checks if FATAL level is enabled, formats the message using std::vformat,
 *             and outputs it with [FATAL] prefix through the console system.
 *
 * @param[in]  fmt   The format string.
 * @param[in]  args  Arguments for string formatting.
 *
 * @tparam     Args  Parameter pack for format arguments.
 *
 * @return     Reference to the logger instance for method chaining.
 */
template <class... Args>
fb::logger& fb::logger::fatal(const std::string& fmt, Args&&... args)
{
    return fatal(fb::console::color::red, fmt, std::forward<Args>(args)...);
}

/**
 * @brief      Template implementation for fatal error logging with color.
 *
 *             Checks if FATAL level is enabled, formats the message using std::vformat,
 *             and outputs it with [FATAL] prefix and specified color through the console system.
 *
 * @param[in]  color  The text color.
 * @param[in]  fmt    The format string.
 * @param[in]  args   Arguments for string formatting.
 *
 * @tparam     Args   Parameter pack for format arguments.
 *
 * @return     Reference to the logger instance for method chaining.
 */
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

/**
 * @brief      Template implementation for comment logging with color.
 *
 *             Outputs the message with specified color through the console system.
 *             Uses std::vformat for string formatting.
 *
 * @param[in]  color  The text color.
 * @param[in]  fmt    The format string.
 * @param[in]  args   Arguments for string formatting.
 *
 * @tparam     Args   Parameter pack for format arguments.
 *
 * @return     Reference to the logger instance for method chaining.
 */
template <class... Args>
fb::logger& fb::logger::comment(fb::console::color color, const std::string& fmt, Args&&... args)
{
    auto& ist     = get();
    auto  message = std::vformat(fmt, std::make_format_args(args...));
    fb::console::comment(color, message);

    return ist;
}

/**
 * @brief      Template implementation for comment logging.
 *
 *             Outputs the message through the console system.
 *             Uses std::vformat for string formatting.
 *
 * @param[in]  fmt   The format string.
 * @param[in]  args  Arguments for string formatting.
 *
 * @tparam     Args  Parameter pack for format arguments.
 *
 * @return     Reference to the logger instance for method chaining.
 */
template <class... Args>
fb::logger& fb::logger::comment(const std::string& fmt, Args&&... args)
{
    return comment(fb::console::color::white, fmt, std::forward<Args>(args)...);
}

#endif // !__LOGGER_H__