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

void tasks::enqueue(const task& t)
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

void worker::enqueue(const task& t)
{
    this->_tasks.enqueue(t);
}

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

            auto&& x = std::move(connection);
            this->_connections.enqueue(x);
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
        _workers.push_back(std::make_unique<worker>(databases[i], pool_size));
        post(boost_thread_pool, [this, i] { this->_workers.at(i)->on_work(); });
    }
}

context::~context()
{ }

uint64_t context::hash(const std::string& name) const
{
    if(name.empty())
        return 0;

    auto hash   = uint64_t(0);
    auto length = name.length();
    for(int i = 0; i < length; i++)
    {
        hash = 31 * hash + name[i];
    }

    return hash;
}

uint8_t context::index(const std::string& name) const
{
    if(name.empty())
        return 0;

    auto size = this->_workers.size();
    if(size == 1)
        return 0;

    return (uint8_t)(this->hash(name) % size - 1) + 1;
}

void context::enqueue(const std::string& name, const task& t)
{
    auto index = this->index(name);
    this->_workers[index]->enqueue(t);
}

context& context::get()
{
    static std::once_flag           flag;
    static std::unique_ptr<context> ist;
    std::call_once(flag, [] { ist.reset(new context(10)); });

    return *ist;
}

std::string fb::db::fstring(const char* fmt, ...) 
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

void fb::db::exec(const std::string& name, const std::string& sql)
{
    auto& ist = context::get();
    ist.enqueue(name, task {sql, [] (auto& results) {}});
}

result_type fb::db::co_exec(const std::string& name, const std::string& sql)
{
    auto await_callback = [name, sql](auto& awaitable)
    {
        auto& ist = context::get();
        ist.enqueue(name, task {sql, [&awaitable] (auto& results) 
        {
            awaitable.result = &results;
            awaitable.handler.resume();
        }});
    };

    return fb::awaitable<std::vector<daotk::mysql::result>>(await_callback);
}

void fb::db::exec(const std::string& name, const std::vector<std::string>& queries)
{
    auto sstream = std::stringstream();
    for (auto& query : queries)
    {
        if (query.empty())
            continue;

        sstream << query << ";";
    }

    exec(name, sstream.str());
}

result_type fb::db::co_exec(const std::string& name, const std::vector<std::string>& queries)
{
    auto sstream = std::stringstream();
    for (auto& query : queries)
    {
        if (query.empty())
            continue;

        sstream << query << ";";
    }

    return co_exec(name, sstream.str());
}