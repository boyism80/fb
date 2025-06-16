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
 * @brief      Utility class for little-endian byte order operations.
 *
 *             Provides static methods to read and write values in little-endian
 *             byte order, handling endianness conversion automatically based on
 *             the target platform's native byte order.
 */
class little_endian
{
public:
    /**
     * @brief      Reads a value from a byte array in little-endian format.
     *
     *             Converts the byte sequence to the specified value type,
     *             handling endianness conversion if the target platform
     *             uses big-endian byte order.
     *
     * @param[in]  p          Pointer to the byte array to read from.
     *
     * @tparam     ValueType  The type of value to read (e.g., uint16_t, uint32_t, int, etc.).
     *
     * @return     The value read from the byte array in native byte order.
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
     * @brief      Writes a value to a byte array in little-endian format.
     *
     *             Converts the value to little-endian byte order and stores
     *             it in the provided byte array, handling endianness conversion
     *             if the target platform uses big-endian byte order.
     *
     * @param[in]  value      The value to write to the byte array.
     * @param      p          Pointer to the byte array to write to.
     *
     * @tparam     ValueType  The type of value to write (e.g., uint16_t, uint32_t, int, etc.).
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
 * @brief      Utility class for big-endian byte order operations.
 *
 *             Provides static methods to read and write values in big-endian
 *             byte order, handling endianness conversion automatically based on
 *             the target platform's native byte order.
 */
class big_endian
{
public:
    /**
     * @brief      Reads a value from a byte array in big-endian format.
     *
     *             Converts the byte sequence to the specified value type,
     *             handling endianness conversion if the target platform
     *             uses little-endian byte order.
     *
     * @param[in]  p          Pointer to the byte array to read from.
     *
     * @tparam     ValueType  The type of value to read (e.g., uint16_t, uint32_t, int, etc.).
     *
     * @return     The value read from the byte array in native byte order.
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
     * @brief      Writes a value to a byte array in big-endian format.
     *
     *             Converts the value to big-endian byte order and stores
     *             it in the provided byte array, handling endianness conversion
     *             if the target platform uses little-endian byte order.
     *
     * @param[in]  value      The value to write to the byte array.
     * @param      p          Pointer to the byte array to write to.
     *
     * @tparam     ValueType  The type of value to write (e.g., uint16_t, uint32_t, int, etc.).
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

} // namespace fb

#endif