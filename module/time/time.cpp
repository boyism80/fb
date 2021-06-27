#include "time.h"

tm* fb::now()
{
    auto now = std::time(nullptr);
    return std::localtime(&now);
}