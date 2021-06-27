#include "time.h"

tm* fb::now()
{
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);

    return std::localtime(&now_c);
}