#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <fb/core/console.h>

namespace fb {

class logger
{
public:
    enum level
    {
        info, debug, warn, fatal
    };

private:
#ifdef _WIN32
    HANDLE                  _stdout;
    int                     _x       = 0;
    int                     _y       = 0;
#endif
    uint16_t                _width   = 0;
    uint16_t                _height  = 0;
    std::mutex              _mutex;

private:
    logger();

public:
    logger(const logger&) = delete;
    logger(logger&&) = delete;
    ~logger();

public:
    logger& operator = (logger&) = delete;
    logger& operator = (const logger&) = delete;

public:
    fb::logger&            put(const char* format, ...);

public:
    static logger&         get();
};

}

#endif // !__LOGGER_H__