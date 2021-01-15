#ifndef __DB_H__
#define __DB_H__

#include "mysql+++/mysql+++.h"
#include "module/config/config.h"
#include <string>
#include <boost/asio.hpp>
#include <future>
#include <functional>
#include <deque>
#include "module/console/console.h"

using namespace daotk::mysql;

namespace fb {

class db
{
private:
    static const uint32_t                         SIZE = 10;
    static std::deque<daotk::mysql::connection*>* _connections;
    static std::deque<daotk::mysql::connection*>* _usings;
    static uint32_t                               _index;
    static boost::asio::io_context*               _context;
    static std::mutex                             _mutex;

private:
    static std::string format_string_vargs(const char* fmt_str, va_list args) 
    {
        std::size_t size = 256;
        std::vector<char> buf(size);

        while (true) {
            int needed = std::vsnprintf(&buf[0], size, fmt_str, args);

            if (needed <= (int)size && needed >= 0)
                return &buf[0];

            size = (needed > 0) ? (needed + 1) : (size * 2);
            buf.resize(size);
        }
    }

    static std::string format_string(const char* fmt_str, ...) 
    {
        va_list vargs;
        va_start(vargs, fmt_str);
        std::string res = format_string_vargs(fmt_str, vargs);
        va_end(vargs);
        return std::move(res);
    }

private:
    static daotk::mysql::connection&    get();
    static bool                         release(daotk::mysql::connection& connection);

public:
    static void                         bind(boost::asio::io_context& context);
    static void                         release();

    template <typename... Values>
    static void query(const std::string& format, Values... values)
    {
        auto future = std::async
        (
            std::launch::async, 
            [] (const std::string& sql)
            {
                auto& connection = db::get();
                connection.query(sql);
                db::release(connection);
            },
            format_string(format.c_str(), std::forward<Values>(values)...)
        );
    }

    template <typename... Values>
    static bool query(std::function<bool(daotk::mysql::connection& connection, daotk::mysql::result&)> callback, const std::string& format, Values... values)
    {
        if(_context == nullptr)
            return false;

        auto future = std::async
        (
            std::launch::async, 
            [callback] (const std::string& sql)
            {
                auto& connection = db::get();
                auto  result = new daotk::mysql::result(connection.query(sql));
                db::release(connection);

                boost::asio::dispatch
                (
                    *_context, 
                    [&connection, callback, result] () 
                    { 
                        callback(connection, *result); 
                        delete result;
                    }
                );
            },
            format_string(format.c_str(), std::forward<Values>(values)...)
        );

        return true;
    }
};

}

#endif // !__DB_H__
