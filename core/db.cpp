#include <fb/core/db.h>

using namespace fb::db;

connections::connections(const Json::Value& config, int pool_size) : _config(config)
{
    for(int i = 0; i < pool_size; i++)
    {
        this->_queue.push(connection_ptr(nullptr));
    }
}

connections::~connections()
{}

void connections::enqueue(std::unique_ptr<daotk::mysql::connection>& conn)
{
    MUTEX_GUARD(this->_mutex);

    this->_queue.push(std::move(conn));
}

connection_ptr connections::dequeue()
{
    MUTEX_GUARD(this->_mutex);

    if (this->_queue.empty())
        return nullptr;

    auto ptr = std::move(this->_queue.front());
    this->_queue.pop();

    if (ptr == nullptr)
    {
        auto& config            = fb::config::get();
        auto  option            = daotk::mysql::connect_options();
        option.server           = this->_config["ip"].asString();
        option.port             = this->_config["port"].asInt();
        option.username         = this->_config["uid"].asString();
        option.password         = this->_config["pwd"].asString();
        option.dbname           = this->_config["name"].asString();
        option.autoreconnect    = true;

        ptr.reset(new daotk::mysql::connection(option));
        auto busy = false;

        while (ptr->is_open() == false)
        {
            busy = true;
            std::this_thread::sleep_for(100ms);
            ptr->open(option);
        }
        ptr->set_server_option(MYSQL_OPTION_MULTI_STATEMENTS_ON);

        if (busy)
        {
            fb::logger::debug("db re-open success");
            busy = false;
        }
    }

    return std::move(ptr);
}


tasks::tasks()
{ }

tasks::~tasks()
{ }

void tasks::enqueue(const task& t)
{
    MUTEX_GUARD(this->_mutex);
    this->_queue.push(std::make_unique<task>(t));
}

std::unique_ptr<task> tasks::dequeue()
{
    MUTEX_GUARD(this->_mutex);
    if(this->_queue.empty())
        return nullptr;

    auto ptr = std::move(this->_queue.front());
    this->_queue.pop();

    return std::move(ptr);
}

worker::worker(const Json::Value& config, int pool_size) : _connections(config, pool_size)
{ }

worker::~worker()
{ }

void worker::enqueue(const task& t)
{
    this->_tasks.enqueue(t);
}

void worker::exit()
{
    this->_exit = true;
}

void worker::on_work()
{
    constexpr auto term = 100ms;
    while(true)
    {
        auto connection = this->_connections.dequeue();
        if(connection == nullptr)
        {
            std::this_thread::sleep_for(term);
            continue;
        }

        auto task = this->_tasks.dequeue();
        if(task == nullptr)
        {
            if(this->_exit)
                break;
            
            std::this_thread::sleep_for(term);
            this->_connections.enqueue(connection);
            continue;
        }

        try
        {
            auto results = connection->mquery(task->sql);
            task->callback(results);
        }
        catch(std::exception& e)
        {
            fb::logger::fatal("query failed : ", task->sql.c_str());
            task->error(e);
        }

        auto&& x = std::move(connection);
        this->_connections.enqueue(x);
    }
}

base_context::base_context(int pool_size)
{
    auto& config = fb::config::get();
    auto& databases = config["database"];
    auto  count = databases["shard"].size();
    auto  global_exists = (databases["global"].isNull() == false);

    this->_thread_pool = std::make_unique<boost::asio::thread_pool>(count + (global_exists ? 1 : 0));
    for (int i = 0; i < count; i++)
    {
        this->_workers.push_back(std::make_unique<worker>(databases["shard"][i], pool_size));
        post(*_thread_pool, [this, i] { this->_workers.at(i)->on_work(); });
    }

    if(global_exists)
    {
        this->_global_worker = std::make_unique<worker>(databases["global"], pool_size);
        post(*_thread_pool, [this] { this->_global_worker->on_work(); });
    }
}

base_context::~base_context()
{
    this->exit();
}

int base_context::index(uint32_t id) const
{
    if(id == -1)
        return -1;

    auto size = this->_workers.size();
    if(size == 1)
        return 0;

    return (int)(id % size - 1) + 1;
}

void base_context::enqueue(uint32_t id, const task& t)
{
    auto index = this->index(id);
    if(index == -1)
    {
        if(this->_global_worker == nullptr)
            throw std::runtime_error("global db not set");

        this->_global_worker->enqueue(t);
    }
    else
    {
        this->_workers[index]->enqueue(t);
    }
}

void base_context::exit()
{
    std::lock_guard<std::mutex>(this->_mutex_exit);

    for(auto& worker : this->_workers)
    {
        worker->exit();
    }

    if(this->_global_worker != nullptr)
    {
        this->_global_worker->exit();
    }

    if (_thread_pool != nullptr)
    {
        this->_thread_pool->join();
        this->_workers.clear();
        this->_global_worker.reset();
        this->_thread_pool.reset();
    }
}

void fb::db::base_context::exec(uint32_t id, const std::string& sql)
{
    this->enqueue(id, task {sql, [] (auto& results) {}, [] (auto& e) {}});
}

fb::db::awaiter fb::db::base_context::co_exec(uint32_t id, const std::string& sql)
{
    auto await_callback = [this, id, sql](auto& awaitable)
    {
        this->enqueue(id, task {sql, [&awaitable] (auto& results) 
        {
            awaitable.result = &results;
            awaitable.handler.resume();
        }, [&awaitable] (auto& e)
        {
            awaitable.error = std::make_unique<std::runtime_error>(e.what());
            awaitable.handler.resume();
        }});
    };

    return fb::awaitable<std::vector<daotk::mysql::result>>(await_callback);
}

void fb::db::base_context::exec(uint32_t id, const std::vector<std::string>& queries)
{
    auto sstream = std::stringstream();
    for (auto& query : queries)
    {
        if (query.empty())
            continue;

        sstream << query << ";";
    }

    this->exec(id, sstream.str());
}

fb::db::awaiter fb::db::base_context::co_exec(uint32_t id, const std::vector<std::string>& queries)
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

void fb::db::base_context::exec_f(uint32_t id, const std::string& format, ...)
{
    va_list args;
    va_start(args, format);
    auto sql = fb::format(format, &args);
    va_end(args);

    this->exec(id, sql);
}

fb::db::awaiter fb::db::base_context::co_exec_f(uint32_t id, const std::string& format, ...)
{
    va_list args;
    va_start(args, format);
    auto sql = fb::format(format, &args);
    va_end(args);
    
    return this->co_exec(id, sql);
}