#ifndef __STAT_H__
#define __STAT_H__

#include <limits>

/**
 * @brief      Template class for managing character statistics with base and buff values.
 *
 *             This template class provides a safe way to handle character statistics
 *             that consist of a base value and temporary buff/debuff modifications.
 *             It automatically handles overflow protection when combining base and
 *             buff values, ensuring the result never exceeds the type's maximum limit.
 *
 *             Key features:
 *             - Separate tracking of base and temporary buff values
 *             - Automatic overflow protection for safe arithmetic
 *             - Generic template design for different numeric types
 *             - Efficient computation of final stat values
 *
 * @tparam     T     The numeric type for the statistic (e.g., uint16_t, uint32_t).
 */
template <typename T>
class stat_value
{
public:
    T base = 0; ///< The base value of the statistic
    T buff = 0; ///< The temporary buff/debuff modifier

public:
    /**
     * @brief      Calculates the final effective value of the statistic.
     *
     *             Combines the base value with the buff modifier, ensuring that
     *             the result does not exceed the maximum value for the type T.
     *             This prevents integer overflow in stat calculations.
     *
     * @return     The effective value (base + buff), capped at the type's maximum.
     */
    T value() const
    {
        auto limit = std::numeric_limits<T>::max();
        if (limit - base < buff)
            return limit;

        return base + buff;
    }
};

#endif