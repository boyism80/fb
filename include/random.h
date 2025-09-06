#ifndef __RANDOM_H__
#define __RANDOM_H__

template <typename T = int>
T random(T min, T max)
{
    static auto device = std::random_device{};
    static auto gen    = std::mt19937(device());

    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

#endif // !__RANDOM_H__