#ifndef	__FORMAT_H__
#define	__FORMAT_H__

#include <memory>
#include <string>
#include <stdarg.h>
#include <stdexcept>

namespace fb {

template<typename ... Args>
std::string format(const std::string& format, Args ... args)
{
    auto size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
    if(size_s <= 0)
    	throw std::runtime_error("Error during formatting.");

    auto size = static_cast<size_t>(size_s);
    auto buffer = std::make_unique<char[]>(size);
    std::snprintf(buffer.get(), size, format.c_str(), args ...);
    return std::string(buffer.get(), buffer.get() + size - 1);
}

std::string format(const std::string& format, va_list* args);

}

#endif