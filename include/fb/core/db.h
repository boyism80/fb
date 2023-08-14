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
    connection_ptr&&            dequeue();
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
    void                        enqueue(const task&& t);
    std::unique_ptr<task>&&     dequeue();
};


class worker
{
private:
    connections                 _connections;
    tasks                       _tasks;

public:
    worker(const Json::Value& config, int pool_size);
    ~worker();

public:
    void                        on_work();
};


class context
{
private:
    static std::unique_ptr<context> _ist;

private:
    workers                     _workers;

public:
    context(int pool_size);
    ~context();

public:
    static context&             get();
};

} }

#include "db.hpp"

#endif // !__DB_H__
