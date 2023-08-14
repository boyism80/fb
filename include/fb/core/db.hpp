#include "db.h"

using namespace fb::db;

std::string fstring(const char* fmt, ...) 
{
    std::size_t size = 256;
    std::vector<char> buf(size);

    va_list vargs;
    va_start(vargs, fmt);
    while (true)
    {
        int needed = std::vsnprintf(&buf[0], size, fmt, vargs);

        if (needed <= (int)size & needed >= 0)
            break;

        size = (needed > 0) ? (needed + 1) : (size * 2);
        buf.resize(size);
    }

    va_end(vargs);
    return std::string(&buf[0]);
}

template <typename... Values>
void fb::db::exec_f(const std::string& name, const std::string& format, Values... values)
{
	auto sql = fstring(format.c_str(), std::forward<Values>(values)...);
	exec(name, sql);
}

template <typename... Values>
result_type fb::db::co_exec_f(const std::string& name, const std::string& format, Values... values)
{
	auto sql = fstring(format.c_str(), std::forward<Values>(values)...);
	return co_exec(name, sql);
}