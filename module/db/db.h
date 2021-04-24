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
    static std::unique_ptr<db>                      _ist;
    static const uint32_t                           SIZE = 10;

public:
    typedef std::deque<daotk::mysql::connection*>   pool;
    typedef std::vector<pool*>                      pools;

private:
    pools                                           _pools;
    boost::asio::io_context*                        _context;
    std::mutex                                      _mutex;

private:
    db();

public:
    db(const db&) = delete;
    db(db&&) = delete;
    ~db();

public:
    db& operator = (db&) = delete;
    db& operator = (const db&) = delete;

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
    uint64_t                            hash(const char* name);
    uint8_t                             index(const char* name);
    fb::db::pool*                       connections(const char* name);
    daotk::mysql::connection&           get(const char* name);
    bool                                release(const char* name, daotk::mysql::connection& connection);

private:
    void                                _exec(const char* name, const std::string& sql);
    void                                _query(const char* name, const std::string& sql, const std::function<void(daotk::mysql::connection&, daotk::mysql::result&)>& callback);
    void                                _mquery(const char* name, const std::string& sql, const std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)>& callback);

private:
    static db&                          get();

public:
    static void                         bind(boost::asio::io_context& context);

    template <typename... Values>
    static void query(const char* name, const std::string& format, Values... values)
    {
        auto& ist = get();
        if(ist._context == nullptr)
            return;

        auto future = std::async
        (
            std::launch::async, 
            std::bind(&db::_exec, &ist, std::placeholders::_1, std::placeholders::_2),
            name,
            format_string(format.c_str(), std::forward<Values>(values)...)
        );
    }

    template <typename... Values>
    static bool query(const char* name, std::function<void(daotk::mysql::connection&, daotk::mysql::result&)> callback, const std::string& format, Values... values)
    {
        auto& ist = get();
        if(ist._context == nullptr)
            return false;

        auto future = std::async
        (
            std::launch::async, 
            std::bind(&db::_query, &ist, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
            name,
            format_string(format.c_str(), std::forward<Values>(values)...),
            callback
        );

        return true;
    }

    template <typename... Values>
    static bool query(const char* name, std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)> callback, const std::string& format, Values... values)
    {
        auto& ist = get();
        if(ist._context == nullptr)
            return false;

        auto future = std::async
        (
            std::launch::async, 
            std::bind(&db::_mquery, &ist, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
            name,
            format_string(format.c_str(), std::forward<Values>(values)...),
            callback
        );

        return true;
    }

    static bool query(const char* name, std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)> callback, const std::vector<std::string>& queries);
    static bool query(const char* name, const std::vector<std::string>& queries);
};

}

#endif // !__DB_H__
