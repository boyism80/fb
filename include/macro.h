#ifndef __DEFAULT_MACRO_H__
#define __DEFAULT_MACRO_H__

#ifndef interface
#define interface struct
#endif

#define _TEXT(k)      fb::model::const_value::string::k
#define READ_LOCK(m)  auto _ = std::shared_lock<std::shared_mutex>(m)
#define WRITE_LOCK(m) auto _ = std::lock_guard<std::shared_mutex>(m)

#include <type_traits>

/**
 * @brief      Bitwise OR operator for enum types.
 *
 * @tparam     T     The type of the enum.
 *
 * @param      flag1  The first flag.
 * @param      flag2  The second flag.
 *
 * @return     The result of the bitwise OR operation.
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator| (T flag1, T flag2)
{
    return static_cast<T>(static_cast<int>(flag1) | static_cast<int>(flag2));
}

/**
 * @brief      Bitwise OR assignment operator for enum types.
 *
 * @tparam     T     The type of the enum.
 *
 * @param      flag1  The first flag.
 * @param      flag2  The second flag.
 *
 * @return     The result of the bitwise OR assignment operation.
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator|= (T& flag1, const T& flag2)
{
    flag1 = static_cast<T>(static_cast<int>(flag1) | static_cast<int>(flag2));
    return flag1;
}

/**
 * @brief      Logical AND operator for enum types.
 *
 * @tparam     T1     The type of the first enum.
 * @tparam     T2     The type of the second enum.
 *
 * @param      flag1  The first flag.
 * @param      flag2  The second flag.
 *
 * @return     The result of the logical AND operation.
 */
template <typename T1,
          typename T2,
          typename = typename std::enable_if<std::is_enum<T1>::value, T1>::type,
          typename = typename std::enable_if<std::is_enum<T2>::value, T2>::type>
constexpr bool operator&& (const T1& flag1, const T2& flag2)
{
    return static_cast<int>(flag1) && static_cast<int>(flag2);
}

/**
 * @brief      Bitwise AND operator for enum types.
 *
 * @tparam     T     The type of the enum.
 *
 * @param      flag1  The first flag.
 * @param      flag2  The second flag.
 *
 * @return     The result of the bitwise AND operation.
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator& (T flag1, T flag2)
{
    return static_cast<T>(static_cast<int>(flag1) & static_cast<int>(flag2));
}

/**
 * @brief      Bitwise AND assignment operator for enum types.
 *
 * @tparam     T     The type of the enum.
 *
 * @param      flag1  The first flag.
 * @param      flag2  The second flag.
 *
 * @return     The result of the bitwise AND assignment operation.
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator&= (T& flag1, const T& flag2)
{
    flag1 = static_cast<T>(static_cast<int>(flag1) & static_cast<int>(flag2));
    return flag1;
}

/**
 * @brief      Bitwise NOT assignment operator for enum types.
 *
 * @tparam     T     The type of the enum.
 *
 * @param      flag  The flag.
 *
 * @return     The result of the bitwise NOT assignment operation.
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator~(T& flag)
{
    flag = static_cast<T>(~static_cast<int>(flag));
    return flag;
}

/**
 * @brief      Checks if a value is in an enum.
 *
 * @tparam     T     The type of the enum.
 *
 * @param      src   The source value.
 * @param      value The value to check.
 *
 * @return     True if the value is in the enum, false otherwise.
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
bool ENUM_IN(T src, T value)
{
    return (static_cast<int>(src) & static_cast<int>(value)) == static_cast<int>(value);
}

#endif