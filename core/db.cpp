#include <fb/core/db.h>

using namespace fb::db;

connections::~connections()
{}

void connections::enqueue(std::unique_ptr<daotk::mysql::connection>& conn)
{
    MUTEX_GUARD(this->_mutex);

    this->_queue.push(std::move(conn));
}

connection_ptr&& connections::dequeue()
{
    MUTEX_GUARD(this->_mutex);

    if (this->_queue.empty())
        return nullptr;

    auto& c = fb::console::get();
    auto& ptr = this->_queue.front();
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

        if (busy)
        {
            c.puts("db re-open success");
            busy = false;
        }
    }

    return std::move(ptr);
}


tasks::tasks()
{ }

tasks::~tasks()
{ }

void tasks::enqueue(const task&& t)
{
    MUTEX_GUARD(this->_mutex);
    this->_queue.push(std::make_unique<task>(t));
}

std::unique_ptr<task>&& tasks::dequeue()
{
    MUTEX_GUARD(this->_mutex);
    if(this->_queue.empty())
        return nullptr;

    auto& ptr = this->_queue.front();
    this->_queue.pop();

    return std::move(ptr);
}

worker::worker(const Json::Value& config, int pool_size) : _connections(config, pool_size)
{ }

worker::~worker()
{ }

void worker::on_work()
{
    constexpr auto term = 100ms;
    while(true)
    {
        auto&& connection = this->_connections.dequeue();
        if(connection == nullptr)
        {
            std::this_thread::sleep_for(term);
            continue;
        }

        auto&& task = this->_tasks.dequeue();
        if(task == nullptr)
        {
            std::this_thread::sleep_for(term);
            this->_connections.enqueue(connection);
            continue;
        }

        std::async([this, connection = std::move(connection), task = std::move(task)] () mutable
        {
            auto results = connection->mquery(task->sql);
            task->callback(results);

            auto&& xxx = std::move(connection);
            this->_connections.enqueue(xxx);
        });
    }
}



context::context(int pool_size)
{
    auto& config = fb::config::get();
    auto& databases = config["database"];
    auto  count = databases.size();
    auto  boost_thread_pool = boost::asio::thread_pool(count);
    for (int i = 0; i < count; i++)
    {
        _workers.push_back(std::make_unique<worker>(pool_size));
        post(boost_thread_pool, [this, i] { this->_workers.at(i)->on_work(); });
    }
}

context::~context()
{ }

context& context::get()
{
    static std::once_flag flag;
    std::call_once(flag, [] { _ist.reset(new context(10)); });

    return *_ist;
}