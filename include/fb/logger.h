#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <fb/console.h>
#include <fb/config.h>
#include <boost/algorithm/string.hpp>

namespace fb {

class logger
{
public:
    enum level
    {
        NONE = 0x0000, 
        DEBUG = 0x0001,
        INFO = 0x0002, 
        WARN = 0x0004, 
        FATAL = 0x0008
    };

private:
    level               _level;

private:
    logger(fb::logger::level level);

public:
    logger(const logger&) = delete;
    logger(logger&&) = delete;
    ~logger() = default;

public:
    logger& operator = (logger&) = delete;
    logger& operator = (const logger&) = delete;

private:
    bool                    has_flag(fb::logger::level level) const;
    static logger&          get();

public:
    static fb::logger&      debug(const char* format, ...);
    static fb::logger&      info(const char* format, ...);
    static fb::logger&      warn(const char* format, ...);
    static fb::logger&      fatal(const char* format, ...);
};

}

#endif // !__LOGGER_H__