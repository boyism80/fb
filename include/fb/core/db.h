#ifndef ENABLE_MYSQL
#error You have to define 'ENABLE_MYSQL'
#else

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
#include <fb/core/acceptor.h>
#include <fb/core/format.h>

namespace fb { namespace db {

class connection;
struct task;
class worker;

using connection_ptr     = std::unique_ptr<daotk::mysql::connection>;
using task_ptr           = std::unique_ptr<task>;
using task_result        = std::vector<daotk::mysql::result>&;
using task_callback_func = std::function<void(std::vector<daotk::mysql::result>&)>;
using task_error_func    = std::function<void(std::exception&)>;
using workers            = std::vector<std::unique_ptr<worker>>;
using task_result_type   = async::task<std::vector<daotk::mysql::result>>;

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
    int                         index(uint32_t id) const;

public:
    void                        enqueue(uint32_t id,  const task& t);
    void                        exit();
    void                        exec(uint32_t id, const std::string& sql);
    fb::db::task_result_type    co_exec(uint32_t id, const std::string& sql);
    void                        exec(uint32_t id, const std::vector<std::string>& queries);
    fb::db::task_result_type    co_exec(uint32_t id, const std::vector<std::string>& queries);
    void                        exec_f(uint32_t id, const std::string& format, ...);
    fb::db::task_result_type    co_exec_f(uint32_t id, const std::string& format, ...);
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
    fb::db::task_result_type co_exec(uint32_t id, const std::string& sql)
    {
        auto promise = std::make_shared<async::task_completion_source<std::vector<daotk::mysql::result>>>();
        auto thread = this->_owner.current_thread();
        auto task = fb::db::task
        {
            /* sql */ sql,
            /* callback */
            [this, thread, promise](auto& results)
            {
                if (thread != nullptr)
                {
                    auto ptr = std::make_shared<std::vector<daotk::mysql::result>>();
                    for (auto& result : results)
                        ptr->push_back(std::move(result));

                    async::awaitable_then(thread->dispatch(), [promise, ptr] (async::awaitable_result<void> result) mutable
                    {
                        promise->set_value(std::move(*ptr.get()));
                    });
                }
                else
                {
                    promise->set_value(std::move(results));
                }
            },
            /* error */
            [promise](auto& e)
            {
                promise->set_exception(std::make_exception_ptr(e));
            }
        };
        this->enqueue(id, task);
        return promise->task();
    }
    fb::db::task_result_type co_exec(const std::string& sql)
    {
        return this->co_exec(-1, sql);
    }
    fb::db::task_result_type co_exec(uint32_t id, const std::vector<std::string>& queries)
    {
        auto sstream = std::stringstream();
        for (auto& query : queries)
        {
            if (query.empty())
                continue;

            sstream << query << ";";
        }

        return this->co_exec(id, sstream.str());
    }
    fb::db::task_result_type co_exec(const std::vector<std::string>& queries)
    {
        return this->co_exec(-1, queries);
    }
    fb::db::task_result_type co_exec_f(uint32_t id, const std::string& format, ...)
    {
        va_list args;
        va_start(args, format);
        auto sql = fb::format(format, &args);
        va_end(args);

        return this->co_exec(id, sql);
    }
    fb::db::task_result_type co_exec_f(const std::string& format, ...)
    {
        va_list args;
        va_start(args, format);
        auto sql = fb::format(format, &args);
        va_end(args);

        return this->co_exec(-1, sql);
    }
};

} }

#endif // !__DB_H__

#endif