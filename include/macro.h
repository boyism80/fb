#ifndef __DEFAULT_MACRO_H__
#define __DEFAULT_MACRO_H__

#ifndef interface
#define interface struct
#endif

#define _TEXT(k)      fb::model::const_value::string::k
#define READ_LOCK(m)  auto _ = std::shared_lock<std::shared_mutex>(m)
#define WRITE_LOCK(m) auto _ = std::lock_guard<std::shared_mutex>(m)

#include <format>
#include <type_traits>
#include <fb/model/datetime.h>

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator| (T flag1, T flag2)
{
    return static_cast<T>(static_cast<int>(flag1) | static_cast<int>(flag2));
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator|= (T& flag1, const T& flag2)
{
    flag1 = static_cast<T>(static_cast<int>(flag1) | static_cast<int>(flag2));
    return flag1;
}

template <typename T1, typename T2, typename = typename std::enable_if<std::is_enum<T1>::value, T1>::type, typename = typename std::enable_if<std::is_enum<T2>::value, T2>::type>
constexpr bool operator&& (const T1& flag1, const T2& flag2)
{
    return static_cast<int>(flag1) && static_cast<int>(flag2);
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator& (T flag1, T flag2)
{
    return static_cast<T>(static_cast<int>(flag1) & static_cast<int>(flag2));
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator&= (T& flag1, const T& flag2)
{
    flag1 = static_cast<T>(static_cast<int>(flag1) & static_cast<int>(flag2));
    return flag1;
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator~(T& flag)
{
    flag = static_cast<T>(~static_cast<int>(flag));
    return flag;
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
bool ENUM_IN(T src, T value)
{
    return (static_cast<int>(src) & static_cast<int>(value)) == static_cast<int>(value);
}

inline std::string baram_time()
{
    static const auto epoch = fb::model::datetime("1996-03-31 00:00:00");
    auto              now   = fb::model::datetime();

    // Calculate time difference in milliseconds
    auto diff     = now - epoch;
    auto total_ms = diff.total_milliseconds();

    // Convert to Baram time units
    // 1 Baram day = 3 real hours = 10800000 milliseconds
    auto baram_days  = total_ms / int64_t{10800000};
    auto baram_year  = static_cast<int>(baram_days / 365) + 1;
    auto remain_days = static_cast<int>(baram_days % 365);
    auto baram_month = std::min(remain_days / 30 + 1, 12);

    auto sstream = std::stringstream{};
    sstream << std::format("바람력 {}년 ", baram_year);
    switch (baram_month)
    {
    case 12:
    case 1:
    case 2:
        sstream << "겨울";
        break;

    case 3:
    case 4:
    case 5:
        sstream << "봄";
        break;

    case 6:
    case 7:
    case 8:
        sstream << "여름";
        break;

    case 9:
    case 10:
    case 11:
        sstream << "가을";
        break;
    }

    return sstream.str();
}

#endif