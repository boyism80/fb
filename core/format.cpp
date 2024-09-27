#include <fb/format.h>

std::string fb::format(const std::string& format, va_list* args)
{
    va_list clone;
    va_copy(clone, *args);
    auto size = vsnprintf(nullptr, 0, format.c_str(), clone) + 1;
    if(size == -1)
        throw std::exception();
    va_end(clone);

    auto buffer = std::vector<char>(size);
    if(vsprintf(buffer.data(), format.c_str(), *args) == -1)
        throw std::exception();

    auto result = std::string(buffer.data());
    return result;
}