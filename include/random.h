#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <random>

template <typename T = int>
T random(T min, T max)
{
    static thread_local auto gen = std::mt19937(std::random_device{}());

    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

#endif // !__RANDOM_H__