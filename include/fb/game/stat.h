#ifndef __STAT_H__
#define __STAT_H__

#include <limits>

template <typename T>
class stat_value
{
public:
    T base = 0;
    T buff = 0;

public:
    T value() const
    {
        auto limit = std::numeric_limits<T>::max();
        if (limit - base < buff)
            return limit;

        return base + buff;
    }
};

#endif