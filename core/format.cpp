#include <fb/core/format.h>

std::string fb::format(const std::string& format, va_list* args)
{
    va_list clone;
    va_copy(clone, *args);
    auto size = vsnprintf(nullptr, 0, format.c_str(), clone) + 1;
    if(size == -1)
        throw std::exception();
    va_end(clone);

    auto buffer = new char[size];
    if(buffer == nullptr)
        throw std::exception();

    if(vsprintf(buffer, format.c_str(), *args) == -1)
        throw std::exception();

    auto result = std::string(buffer);
    delete[] buffer;

    return result;
}