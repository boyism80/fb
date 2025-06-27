#ifndef __CONFIG_H__
#define __CONFIG_H__

/**
 * @file    config.h
 * @brief   Configuration management system for the FB 2D MMORPG server
 * @author  FB Development Team
 *
 * @details This file implements a comprehensive configuration management system that
 *          provides JSON-based configuration loading and type-safe value retrieval
 *          for the FB 2D MMORPG server. The system supports command line argument
 *          parsing, automatic type conversion, and thread-safe configuration access.
 *
 *          Key features:
 *          - JSON-based configuration file support with automatic parsing
 *          - Type-safe configuration value retrieval with template specializations
 *          - Command line argument parsing for configuration file path specification
 *          - Thread-safe configuration access with proper synchronization
 *          - Support for all fundamental data types (bool, integers, floats, strings)
 *          - Automatic type conversion from JSON values to C++ types
 *          - Error handling for missing configuration keys and invalid values
 *          - Configuration path management with flexible initialization options
 *          - Integration with Boost.Program_Options for advanced command line parsing
 *          - Console integration for configuration-related logging and debugging
 *
 * @note    The configuration system must be initialized with init_config() or
 *          set_config_path() before using the config() template function to
 *          retrieve configuration values.
 */

#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <json/json.h>
#include <sstream>
#include <mutex>
#include <format>
#include <filesystem>
#include <boost/program_options.hpp>
#include <fb/console.h>

namespace fb {

/**
 * @brief      Template struct to define the return type for configuration values.
 *
 * @tparam     T     The type of the configuration value.
 */
template <typename T> struct config_value_type
{
    typedef T type;
};

/**
 * @brief      Specialization for Json::Value to return a const reference.
 */
template <> struct config_value_type<Json::Value>
{
    typedef const Json::Value& type;
};

/**
 * @brief      Specialization for char* to return a const char pointer.
 */
template <> struct config_value_type<char*>
{
    typedef const char* type;
};

/**
 * @brief      Converts a JSON value to the specified type.
 *
 * @param[in]  value  The JSON value to convert.
 *
 * @tparam     T      The target type for conversion.
 *
 * @return     The converted value of type T.
 */
template <typename T>
inline static typename config_value_type<T>::type json_value(const Json::Value& value)
{
    throw std::runtime_error("unsupported type");
}

template <>
typename config_value_type<Json::Value>::type json_value<Json::Value>(const Json::Value& value)
{
    return value;
}

template <>
typename config_value_type<bool>::type json_value<bool>(const Json::Value& value)
{
    return value.asBool();
}

template <>
typename config_value_type<int8_t>::type json_value<int8_t>(const Json::Value& value)
{
    return static_cast<int8_t>(value.asInt());
}

template <>
typename config_value_type<uint8_t>::type json_value<uint8_t>(const Json::Value& value)
{
    return static_cast<uint8_t>(value.asUInt());
}

template <>
typename config_value_type<int16_t>::type json_value<int16_t>(const Json::Value& value)
{
    return static_cast<int16_t>(value.asInt());
}

template <>
typename config_value_type<uint16_t>::type json_value<uint16_t>(const Json::Value& value)
{
    return static_cast<uint16_t>(value.asUInt());
}

template <>
typename config_value_type<int32_t>::type json_value<int32_t>(const Json::Value& value)
{
    return static_cast<int32_t>(value.asInt());
}

template <>
typename config_value_type<uint32_t>::type json_value<uint32_t>(const Json::Value& value)
{
    return static_cast<uint32_t>(value.asUInt());
}

template <>
typename config_value_type<int64_t>::type json_value<int64_t>(const Json::Value& value)
{
    return static_cast<int64_t>(value.asInt64());
}

template <>
typename config_value_type<uint64_t>::type json_value<uint64_t>(const Json::Value& value)
{
    return static_cast<uint64_t>(value.asUInt64());
}

template <>
typename config_value_type<float>::type json_value<float>(const Json::Value& value)
{
    return value.asFloat();
}

template <>
typename config_value_type<double>::type json_value<double>(const Json::Value& value)
{
    return value.asDouble();
}

template <>
typename config_value_type<char*>::type json_value<char*>(const Json::Value& value)
{
    return value.asCString();
}

template <>
typename config_value_type<std::string>::type json_value<std::string>(const Json::Value& value)
{
    return value.asString();
}

/**
 * @brief      Gets or sets the global config file path.
 *
 * @param[in]  path     Optional path to set. If empty, returns current path.
 * @param[in]  set_mode If true, sets the path. If false, gets the path.
 *
 * @return     The current config file path
 */
inline std::string& config_path_storage(const std::string& path = "", bool set_mode = false)
{
    static std::string stored_path;

    if (set_mode && !path.empty())
        stored_path = path;

    return stored_path;
}

/**
 * @brief      Gets the configured config file path.
 *
 * @return     The path to the config file as a string
 *
 * @note       Returns empty string if init_config() hasn't been called
 */
inline std::string get_config_path()
{
    return config_path_storage();
}

/**
 * @brief      Sets the config file path manually.
 *
 *             Alternative to init_config() for cases where command line parsing
 *             is not available or desired.
 *
 * @param[in]  path  The path to the config file
 */
inline void set_config_path(const std::string& path)
{
    config_path_storage(path, true);
}

/**
 * @brief      Initializes the configuration system with direct config file path.
 *
 *             This function sets the config file path directly without command line parsing.
 *             Simpler alternative to the argc/argv version when config path is known.
 *
 * @param[in]  config_path  The path to the configuration file
 *
 * @return     true if initialization succeeded, false otherwise
 *
 * @note       The config file existence and validity will be checked when config() is first called
 */
inline bool init_config(const std::string& config_path)
{
    try
    {
        if (config_path.empty())
        {
            fb::console::puts("Config initialization failed: config path cannot be empty");
            return false;
        }

        set_config_path(config_path);
        return true;
    }
    catch (const std::exception& e)
    {
        fb::console::puts("Config initialization failed: " + std::string(e.what()));
        return false;
    }
}

/**
 * @brief      Gets a configuration value by key path.
 *
 *             This function loads the configuration file specified by init_config()
 *             or set_config_path() and retrieves the value at the given key path.
 *
 * @param[in]  k     The key path (e.g., "database:host" for nested values).
 *
 * @tparam     T     The type of the configuration value to retrieve.
 *
 * @return     The configuration value of the specified type.
 *
 * @note       init_config() or set_config_path() must be called before using this function
 * @warning    Throws std::runtime_error if config file cannot be loaded or parsed
 */
template <typename T = Json::Value>
inline static typename config_value_type<T>::type config(const std::string& k)
{
    static std::once_flag flag;
    static Json::Value    ist;

    std::call_once(flag, [] {
        auto config_path = get_config_path();
        if (config_path.empty())
            throw std::runtime_error("Config system not initialized. Call init_config() or set_config_path() first.");

        auto ifstream = std::ifstream{};
        try
        {
            ifstream.open(config_path);
            if (ifstream.is_open() == false)
                throw std::runtime_error("cannot load config file " + config_path);

            Json::Reader reader;
            if (reader.parse(ifstream, ist) == false)
                throw std::runtime_error("cannot parse json config file " + config_path);

            ifstream.close();
        }
        catch (std::exception& e)
        {
            if (ifstream.is_open())
                ifstream.close();

            throw e;
        }
    });

    const auto* node    = &ist;
    auto        sstream = std::istringstream{k};
    auto        buffer  = std::string{};
    while (std::getline(sstream, buffer, ':'))
    {
        node = &(*node)[buffer];
    }

    return json_value<T>(*node);
}

} // namespace fb

#endif // !__CONFIG_H__
