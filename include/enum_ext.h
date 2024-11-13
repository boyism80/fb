#ifndef __ENUM_EXT_H__
#define __ENUM_EXT_H__

#include <type_traits>

/**
 * @brief      Bitwise 'or' operator.
 *
 * @param[in]  flag1      The flag 1
 * @param[in]  flag2      The flag 2
 *
 * @tparam     T          { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     The result of the bitwise 'or'
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator| (T flag1, T flag2)
{
    return static_cast<T>(static_cast<int>(flag1) | static_cast<int>(flag2));
}

/**
 * @brief      Bitwise 'or' assignment operator.
 *
 * @param      flag1      The flag 1
 * @param[in]  flag2      The flag 2
 *
 * @tparam     T          { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     The result of the bitwise 'or' assignment
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator|= (T& flag1, const T& flag2)
{
    flag1 = static_cast<T>(static_cast<int>(flag1) | static_cast<int>(flag2));
    return flag1;
}

/**
 * @brief      Logical 'and' operator.
 *
 * @param[in]  flag1      The flag 1
 * @param[in]  flag2      The flag 2
 *
 * @tparam     T1         { description }
 * @tparam     T2         { description }
 * @tparam     <unnamed>  { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     The result of the logical 'and'
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
 * @brief      Bitwise 'and' operator.
 *
 * @param[in]  flag1      The flag 1
 * @param[in]  flag2      The flag 2
 *
 * @tparam     T          { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     The result of the bitwise 'and'
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator& (T flag1, T flag2)
{
    return static_cast<T>(static_cast<int>(flag1) & static_cast<int>(flag2));
}

/**
 * @brief      Bitwise 'and' assignment operator.
 *
 * @param      flag1      The flag 1
 * @param[in]  flag2      The flag 2
 *
 * @tparam     T          { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     The result of the bitwise 'and' assignment
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator&= (T& flag1, const T& flag2)
{
    flag1 = static_cast<T>(static_cast<int>(flag1) & static_cast<int>(flag2));
    return flag1;
}

/**
 * @brief      Bitwise 'one's complement' operator.
 *
 * @param      flag       The flag
 *
 * @tparam     T          { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     The result of the bitwise 'one's complement'
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
constexpr T operator~(T& flag)
{
    flag = static_cast<T>(~static_cast<int>(flag));
    return flag;
}

/**
 * @brief      { function_description }
 *
 * @param[in]  src        The source
 * @param[in]  value      The value
 *
 * @tparam     T          { description }
 * @tparam     <unnamed>  { description }
 *
 * @return     { description_of_the_return_value }
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
bool enum_in(T src, T value)
{
    return (static_cast<int>(src) & static_cast<int>(value)) == static_cast<int>(value);
}

#endif