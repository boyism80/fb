#ifndef __DEFAULT_MACRO_H__
#define __DEFAULT_MACRO_H__

#ifndef interface
#define interface struct
#endif

#define _TEXT(k) fb::model::const_value::string::k
#define READ_LOCK(m)    auto _ = std::shared_lock<std::shared_mutex>(m)
#define WRITE_LOCK(m)   auto _ = std::lock_guard<std::shared_mutex>(m)

#include <type_traits>

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

template <typename T1,
          typename T2,
          typename = typename std::enable_if<std::is_enum<T1>::value, T1>::type,
          typename = typename std::enable_if<std::is_enum<T2>::value, T2>::type>
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

#endif