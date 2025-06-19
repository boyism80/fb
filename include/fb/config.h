#ifndef __CONFIG_H__
#define __CONFIG_H__

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
 * @brief      Initializes the configuration system with command line arguments.
 *
 *             This function must be called from main() before using config() function.
 *             It parses command line arguments to determine the config file path.
 *
 * @param[in]  argc  Command line argument count
 * @param[in]  argv  Command line argument values
 *
 * @return     true if initialization succeeded, false otherwise
 *
 * @note       If no config file is specified, defaults to config.json in executable directory
 */
inline bool init_config(int argc, char* argv[])
{
    namespace po = boost::program_options;

    try
    {
        po::options_description desc("Configuration options");
        desc.add_options()("config,c", po::value<std::string>(), "Path to configuration file");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        // Preferred initialization pattern with default value
        auto config_path_value = std::string{};
        if (vm.count("config"))
            config_path_value = vm["config"].as<std::string>();
        else
        {
            auto exe_path     = std::filesystem::path(argv[0]).parent_path();
            config_path_value = (exe_path / "config.json").string();
        }

        set_config_path(config_path_value);

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

            fb::console::puts(e.what());
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
