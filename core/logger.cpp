#include <fb/core/logger.h>

fb::logger::logger(fb::logger::level level) : _level(level)
{ }

bool fb::logger::has_flag(fb::logger::level level) const
{
    return (this->_level & level) == level;
}

fb::logger& fb::logger::get()
{
    static std::once_flag               flag;
    static std::unique_ptr<fb::logger>  ist;

    std::call_once(flag, [] 
    {
        auto& config = fb::config::get();
        auto level = (int)fb::logger::level::NONE;
        for(auto& x : config["log"])
        {
            auto value = boost::algorithm::to_lower_copy(x.asString());
            if(value == "debug")
                level |= (int)fb::logger::level::DEBUG;
            else if(value == "info")
                level |= (int)fb::logger::level::INFO;
            else if(value == "warn")
                level |= (int)fb::logger::level::WARN;
            else if(value == "fatal")
                level |= (int)fb::logger::level::FATAL;
        }
        ist = std::unique_ptr<fb::logger>(new fb::logger((fb::logger::level)level));
    });
    return *ist;
}

fb::logger& fb::logger::debug(const char* format, ...)
{
    auto& ist = get();
    if(ist.has_flag(fb::logger::level::DEBUG) == false)
        return ist;

    va_list args;
    va_start(args, format);
    auto message = fstring_c(format, &args);
    va_end(args);

    auto& c = fb::console::get();
    c.puts("[DEBUG] %s", message.c_str());

    return ist;
}

fb::logger& fb::logger::info(const char* format, ...)
{
    auto& ist = get();
    if(ist.has_flag(fb::logger::level::INFO) == false)
        return ist;

    va_list args;
    va_start(args, format);
    auto message = fstring_c(format, &args);
    va_end(args);

    auto& c = fb::console::get();
    c.puts("[INFO] %s", message.c_str());

    return ist;
}

fb::logger& fb::logger::warn(const char* format, ...)
{
    auto& ist = get();
    if(ist.has_flag(fb::logger::level::WARN) == false)
        return ist;

    va_list args;
    va_start(args, format);
    auto message = fstring_c(format, &args);
    va_end(args);

    auto& c = fb::console::get();
    c.puts("[WARN] %s", message.c_str());

    return ist;
}

fb::logger& fb::logger::fatal(const char* format, ...)
{
    auto& ist = get();
    if(ist.has_flag(fb::logger::level::FATAL) == false)
        return ist;

    va_list args;
    va_start(args, format);
    auto message = fstring_c(format, &args);
    va_end(args);

    auto& c = fb::console::get();
    c.puts("[FATAL] %s", message.c_str());

    return ist;
}