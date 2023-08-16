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

namespace fb { namespace db {

class connection;
class task;
class worker;

using connection_ptr = std::unique_ptr<daotk::mysql::connection>;
using task_ptr = std::unique_ptr<task>;
using task_result = std::vector<daotk::mysql::result>&;
using task_callback_func = std::function<void(std::vector<daotk::mysql::result>&)>;
using workers = std::vector<std::unique_ptr<worker>>;
using result_type = fb::awaitable<std::vector<daotk::mysql::result>>;

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


class context
{
private:
    using thread_pool = std::unique_ptr<boost::asio::thread_pool>;

private:
    workers                     _workers;
    thread_pool                 _thread_pool;

public:
    context(int pool_size);
    ~context();

private:
    uint64_t                    hash(const std::string& name) const;
    uint8_t                     index(const std::string& name) const;

public:
    void                        enqueue(const std::string& name,  const task& t);
    void                        exit();

    void                        init();
    void                        exec(const std::string& name, const std::string& sql);
    result_type                 co_exec(const std::string& name, const std::string& sql);

    void                        exec(const std::string& name, const std::vector<std::string>& queries);
    result_type                 co_exec(const std::string& name, const std::vector<std::string>& queries);

    template <typename... Values>
    void                        exec_f(const std::string& name, const std::string& format, Values... values);

    template <typename... Values>
    result_type                 co_exec_f(const std::string& name, const std::string& format, Values... values);
};

std::string                     fstring(const char* fmt, ...);

} }

#include "db.hpp"

#endif // !__DB_H__
