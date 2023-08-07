#include "db.h"

template <typename... Values>
void fb::db::query(const std::string& name, const std::string& format, Values... values)
{
    auto& ist = get();
    if(ist._context == nullptr)
        return;

    auto fmt = fstring(format.c_str(), std::forward<Values>(values)...);
    ist._exec(name, fmt);
}

template <typename... Values>
void fb::db::async_query(const std::string& name, const std::string& format, Values... values)
{
    fb::async::launch([name, format, &values...] 
    {
        query(name, format, std::forward<Values>(values)...);
    });
}

template <typename... Values>
void fb::db::query(const std::string& name, const std::function<void(daotk::mysql::connection&, daotk::mysql::result&)>& callback, const std::string& format, Values... values)
{
    auto& ist = get();
    if(ist._context == nullptr)
        return;

    auto fmt = fstring(format.c_str(), std::forward<Values>(values)...);
    ist._query(name, fmt, callback);
}

template <typename... Values>
void fb::db::async_query(const std::string& name, const std::function<void(daotk::mysql::connection&, daotk::mysql::result&)>& callback, const std::string& format, Values... values)
{
    fb::async::launch([name, callback, format, &values...] 
    {
        query(name, callback, format, std::forward<Values>(values)...);
    });
}

template <typename... Values>
auto fb::db::co_query_once(const std::string& name, const std::string& format, Values... values)
{
    auto await_callback = [name, format, &values...](fb::awaitable<daotk::mysql::result>& awaitable)
    {
        auto db_callback = [&awaitable](daotk::mysql::connection& connection, daotk::mysql::result& result)
        {
            awaitable.result = &result;
            awaitable.handler.resume();
        };

        async_query(name, db_callback, format, std::forward<Values>(values)...);
    };

    return fb::awaitable<daotk::mysql::result>(await_callback);
}

template <typename... Values>
void fb::db::query(const std::string& name, const std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)>& callback, const std::string& format, Values... values)
{
    auto& ist = get();
    if(ist._context == nullptr)
        return;

    auto fmt = fstring(format.c_str(), std::forward<Values>(values)...);
    ist._mquery(name, fmt, callback);
}

template <typename... Values>
void fb::db::async_query(const std::string& name, const std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)>& callback, const std::string& format, Values... values)
{
    fb::async::launch([name, callback, format, &values...]
    {
        query(name, callback, format, std::forward<Values>(values)...);
    });
}

template <typename... Values>
auto fb::db::co_query(const std::string& name, const std::string& format, Values... values)
{
    auto await_callback = [name, format, &values...](fb::awaitable<std::vector<daotk::mysql::result>>& awaitable)
    {
        auto db_callback = [&awaitable](daotk::mysql::connection& connection, std::vector<daotk::mysql::result>& results)
        {
            awaitable.result = &results;
            awaitable.handler.resume();
        };

        return async_query(name, db_callback, format, std::forward<Values>(values)...);
    };

    return fb::awaitable<std::vector<daotk::mysql::result>>(await_callback);
}