#ifndef __DB_H__
#define __DB_H__

#include "mysql+++/mysql+++.h"
#include <fb/core/config.h>
#include <fb/core/thread.h>
#include <string>
#include <boost/asio.hpp>
#include <future>
#include <functional>
#include <deque>
#include <fb/core/console.h>
#include <fb/core/coroutine.h>

using namespace daotk::mysql;

namespace fb {

class db
{
private:
    static std::unique_ptr<db>                      _ist;
    static constexpr uint32_t                       SIZE = 2;

public:
    using connection = std::unique_ptr<daotk::mysql::connection>;
    using pool       = std::deque<std::unique_ptr<daotk::mysql::connection>>;
    using pools      = std::vector<std::unique_ptr<pool>>;

private:
    pools                                           _pools;
    boost::asio::io_context*                        _context;
    std::mutex                                      _mutex;

private:
    db();

public:
    db(const db&) = delete;
    db(db&) = delete;
    ~db();

public:
    db& operator = (db&) = delete;
    db& operator = (const db&) = delete;

private:
    static std::string fstring(const char* fmt, ...) 
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

private:
    uint64_t                            hash(const std::string& name);
    uint8_t                             index(const std::string& name);
    fb::db::pool*                       connections(const std::string& name);
    fb::db::connection                  get(const std::string& name);
    bool                                release(const std::string& name, fb::db::connection& connection);

private:
    void                                _exec(const std::string& name, const std::string& sql);
    void                                _query(const std::string& name, const std::string& sql, const std::function<void(daotk::mysql::connection&, daotk::mysql::result&)>& fn);
    void                                _mquery(const std::string& name, const std::string& sql, const std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)>& fn);

private:
    static db&                          get();

public:
    static void                         bind(boost::asio::io_context& context);

public:
    template <typename... Values>
    static void                         query(const std::string& name, const std::string& format, Values... values);
    template <typename... Values>
    static void                         async_query(const std::string& name, const std::string& format, Values... values);
    template <typename... Values>
    static void                         query(const std::string& name, const std::function<void(daotk::mysql::connection&, daotk::mysql::result&)>& callback, const std::string& format, Values... values);
    template <typename... Values>
    static void                         async_query(const std::string& name, const std::function<void(daotk::mysql::connection&, daotk::mysql::result&)>& callback, const std::string& format, Values... values);
    template <typename... Values>
    static auto                         co_query_once(const std::string& name, const std::string& format, Values... values);
    template <typename... Values>
    static void                         query(const std::string& name, const std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)>& callback, const std::string& format, Values... values);
    template <typename... Values>
    static void                         async_query(const std::string& name, const std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)>& callback, const std::string& format, Values... values);
    template <typename... Values>
    static auto                         co_query(const std::string& name, const std::string& format, Values... values);
    static void                         query(const std::string& name, const std::vector<std::string>& queries);
    static void                         async_query(const std::string& name, const std::vector<std::string>& queries);
    static void                         query(const std::string& name, const std::function<void()>& callback, const std::vector<std::string>& queries);
    static void                         async_query(const std::string& name, const std::function<void()>& callback, const std::vector<std::string>& queries);
    static auto                         co_query(const std::string& name, const std::vector<std::string>& queries);
};

}

#include "db.hpp"

#endif // !__DB_H__
