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
#include <fb/core/logger.h>
#include <fb/core/coroutine.h>
#include <fb/core/acceptor.h>

namespace fb { namespace db {

class connection;
class task;
class worker;

using connection_ptr     = std::unique_ptr<daotk::mysql::connection>;
using task_ptr           = std::unique_ptr<task>;
using task_result        = std::vector<daotk::mysql::result>&;
using task_callback_func = std::function<void(std::vector<daotk::mysql::result>&)>;
using task_error_func    = std::function<void(std::exception&)>;
using workers            = std::vector<std::unique_ptr<worker>>;
using result_type        = fb::awaitable<std::vector<daotk::mysql::result>>;

class connections
{
private:
    const Json::Value&          _config;
    std::queue<connection_ptr>  _queue;
    std::mutex                  _mutex;

public:
    connections(const Json::Value& config, int pool_size);
    ~connections();

public:
    void                        enqueue(std::unique_ptr<daotk::mysql::connection>& conn);
    connection_ptr              dequeue();
};


struct task
{
public:
    std::string                 sql;
    task_callback_func          callback;
    task_error_func             error;

};

class tasks
{
private:
    std::queue<task_ptr>        _queue;
    std::mutex                  _mutex;

public:
    tasks();
    ~tasks();

public:
    void                        enqueue(const task& t);
    std::unique_ptr<task>       dequeue();
};


class worker
{
private:
    connections                 _connections;
    tasks                       _tasks;
    bool                        _exit = false;

public:
    worker(const Json::Value& config, int pool_size);
    ~worker();

public:
    void                        enqueue(const task& t);
    void                        exit();

public:
    void                        on_work();
};


class base_context
{
private:
    using thread_pool = std::unique_ptr<boost::asio::thread_pool>;

private:
    workers                     _workers;
    std::unique_ptr<worker>     _global_worker;
    thread_pool                 _thread_pool;
    std::mutex                  _mutex_exit;

public:
    base_context(int pool_size);
    virtual ~base_context();

private:
    uint64_t                    hash(const std::string& name) const;
    int                        index(const std::string& name) const;

public:
    void                        enqueue(const std::string& name,  const task& t);
    void                        exit();
    void                        exec(const std::string& name, const std::string& sql);
    result_type                 co_exec(const std::string& name, const std::string& sql);
    void                        exec(const std::string& name, const std::vector<std::string>& queries);
    result_type                 co_exec(const std::string& name, const std::vector<std::string>& queries);
    void                        exec_f(const std::string& name, const std::string& format, ...);
    result_type                 co_exec_f(const std::string& name, const std::string& format, ...);
};

template <typename T>
class context : base_context
{
private:
    fb::acceptor<T>& _owner;

public:
    context(fb::acceptor<T>& owner, int pool_size) : fb::db::base_context(pool_size), _owner(owner)
    {}
    ~context()
    {}

public:
    using fb::db::base_context::exit;

public:
    fb::db::result_type     co_exec(const std::string& name, const std::string& sql)
    {
        auto await_callback = [this, sql, name](auto& awaitable)
        {
            auto thread = this->_owner.current_thread();
            this->enqueue(name, fb::db::task{ sql, [this, thread, &awaitable](auto& results)
            {
                auto data = std::make_shared<std::vector<daotk::mysql::result>>();
                for(auto& x : results)
                {
                    data->push_back(std::move(x));
                }

                if(thread != nullptr)
                {
                    thread->dispatch([&awaitable, data](uint8_t) mutable
                    {
                        awaitable.result = data.get();
                        awaitable.handler.resume();
                    });
                }
                else
                {
                    awaitable.result = data.get();
                    awaitable.handler.resume();
                }
            }, [&awaitable](auto& e)
            {
                awaitable.error = std::make_unique<std::runtime_error>(e.what());
                awaitable.handler.resume();
            } });
        };
        return fb::awaitable<std::vector<daotk::mysql::result>>(await_callback);
    }
    fb::db::result_type     co_exec(const std::string& name, const std::vector<std::string>& queries)
    {
        auto sstream = std::stringstream();
        for (auto& query : queries)
        {
            if (query.empty())
                continue;

            sstream << query << ";";
        }

        return this->co_exec(name, sstream.str());
    }
    fb::db::result_type     co_exec_f(const std::string& name, const std::string& format, ...)
    {
        va_list args;
        va_start(args, format);
        auto sql = fstring_c(format, &args);
        va_end(args);

        return this->co_exec(name, sql);
    }
};

} }

#endif // !__DB_H__
