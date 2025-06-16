#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <fb/config.h>
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
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::DEBUG) == false)
        return ist;

    auto message = std::vformat(fmt, std::make_format_args(args...));
    fb::console::puts("[DEBUG] {}", message);

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
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::INFO) == false)
        return ist;

    auto message = std::vformat(fmt, std::make_format_args(args...));
    fb::console::puts("[INFO] {}", message);

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
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::WARN) == false)
        return ist;

    auto message = std::vformat(fmt, std::make_format_args(args...));
    fb::console::puts("[WARN] {}", message);

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
    auto& ist = get();
    if (ist.has_flag(fb::logger::level::FATAL) == false)
        return ist;

    auto message = std::vformat(fmt, std::make_format_args(args...));
    fb::console::puts("[FATAL] {}", message);

    return ist;
}

#endif // !__LOGGER_H__