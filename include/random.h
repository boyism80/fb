#ifndef __RANDOM_H__
#define __RANDOM_H__

/**
 * @brief      Generates a random number between a minimum and maximum value.
 *
 * @tparam     T     The type of the random number.
 *
 * @param      min   The minimum value.
 * @param      max   The maximum value.
 *
 * @return     A random number between the minimum and maximum values.
 */
template <typename T = int>
T random(T min, T max)
{
    static auto device = std::random_device{};
    static auto gen    = std::mt19937(device());

    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

#endif // !__RANDOM_H__