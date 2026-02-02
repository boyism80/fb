#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <string_view>
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

template <typename T> struct config_value_type
{
    typedef T type;
};

template <> struct config_value_type<Json::Value>
{
    typedef const Json::Value& type;
};

template <> struct config_value_type<char*>
{
    typedef const char* type;
};

template <> struct config_value_type<std::string_view>
{
    typedef std::string_view type;
};

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

template <>
typename config_value_type<std::string_view>::type json_value<std::string_view>(const Json::Value& value)
{
    // Json::Value stores string internally, asCString() returns pointer to internal storage
    // The static ist in config() ensures the Json::Value lifetime
    auto cstr = value.asCString();
    auto str  = value.asString();
    return std::string_view(cstr, str.size());
}

inline std::string& config_path_storage(std::string_view path = "", bool set_mode = false)
{
    static std::string stored_path;

    if (set_mode && !path.empty())
        stored_path = std::string(path);

    return stored_path;
}

inline std::string get_config_path()
{
    return config_path_storage();
}

inline void set_config_path(std::string_view path)
{
    config_path_storage(path, true);
}

inline bool init_config(std::string_view config_path)
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

template <typename T = Json::Value>
inline static typename config_value_type<T>::type config(std::string_view k)
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
    auto        sstream = std::istringstream{std::string(k)};
    auto        buffer  = std::string{};
    while (std::getline(sstream, buffer, ':'))
    {
        node = &(*node)[buffer];
    }

    return json_value<T>(*node);
}

} // namespace fb

#endif // !__CONFIG_H__
