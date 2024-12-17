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

template <typename T = Json::Value>
inline static typename config_value_type<T>::type config(const std::string& k)
{
    static std::once_flag flag;
    static Json::Value    ist;

    std::call_once(flag, [] {
        const char* env = std::getenv("KINGDOM_OF_WIND_ENVIRONMENT");
#if defined DEBUG || defined _DEBUG
        if (env == nullptr)
            env = "dev";
#endif

        auto ifstream = std::ifstream{};
        try
        {
            auto path = env != nullptr ? std::format("config/config.{}.json", env) : "config/config.json";
            ifstream.open(path);
            if (ifstream.is_open() == false)
            {
                throw std::runtime_error(std::format("cannot load file {}", path));
            }

            Json::Reader reader;
            if (reader.parse(ifstream, ist) == false)
            {
                throw std::runtime_error(std::format("cannot parse json file {}", path));
            }

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
