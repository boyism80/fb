#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <cstring> // std::memcpy

#if defined(__s390x__)
#define LITTLE_ENDIAN 0
#endif // __s390x__
#if !defined(LITTLE_ENDIAN)
#if defined(__GNUC__) || defined(__clang__) || defined(__ICCARM__)
#if (defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__))
#define LITTLE_ENDIAN 0
#else
#define LITTLE_ENDIAN 1
#endif // __BIG_ENDIAN__
#elif defined(_MSC_VER)
#if defined(_M_PPC)
#define LITTLE_ENDIAN 0
#else
#define LITTLE_ENDIAN 1
#endif
#else
#error Unable to determine endianness, define LITTLE_ENDIAN.
#endif
#endif // !defined(LITTLE_ENDIAN)

namespace fb {

/**
 * @brief      This class describes a little endian.
 */
class little_endian
{
public:
    /**
     * @brief      Gets the specified p.
     *
     * @param[in]  p          { parameter_description }
     *
     * @tparam     ValueType  { description }
     *
     * @return     The value type.
     */
    template <typename ValueType>
    static ValueType get(const uint8_t* p)
    {
        auto value = ValueType();
        auto ptr   = (uint8_t*)(ValueType*)&value;
#ifndef LITTLE_ENDIAN
        for (int i = 0; i < sizeof(ValueType); i++)
        {
            ptr[sizeof(ValueType) - i - 1] = p[i];
        }
#else
        std::memcpy(ptr, p, sizeof(ValueType));
#endif
        return value;
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  value      The value
     * @param      p          { parameter_description }
     *
     * @tparam     ValueType  { description }
     */
    template <typename ValueType>
    static void put(const ValueType& value, uint8_t* p)
    {
        auto ptr = (uint8_t*)(ValueType*)&value;
        for (int i = 0; i < sizeof(ValueType); i++)
        {
#ifdef LITTLE_ENDIAN
            p[i] = ptr[i];
#else
            p[sizeof(ValueType) - i - 1] = ptr[i];
#endif
        }
    }
};

/**
 * @brief      This class describes a big endian.
 */
class big_endian
{
public:
    /**
     * @brief      Gets the specified p.
     *
     * @param[in]  p          { parameter_description }
     *
     * @tparam     ValueType  { description }
     *
     * @return     The value type.
     */
    template <typename ValueType>
    static ValueType get(const uint8_t* p)
    {
        auto value = ValueType();
        auto ptr   = (uint8_t*)(ValueType*)&value;
#ifdef LITTLE_ENDIAN
        for (int i = 0; i < sizeof(ValueType); i++)
        {
            ptr[sizeof(ValueType) - i - 1] = p[i];
        }
#else
        std::memcpy(ptr, p, sizeof(ValueType));
#endif
        return value;
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  value      The value
     * @param      p          { parameter_description }
     *
     * @tparam     ValueType  { description }
     */
    template <typename ValueType>
    static void put(const ValueType& value, uint8_t* p)
    {
        auto ptr = (uint8_t*)(ValueType*)&value;
        for (int i = 0; i < sizeof(ValueType); i++)
        {
#ifndef LITTLE_ENDIAN
            p[i] = ptr[i];
#else
            p[sizeof(ValueType) - i - 1] = ptr[i];
#endif
        }
    }
};

}

#endif