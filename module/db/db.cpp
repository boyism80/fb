#include "db.h"

std::unique_ptr<fb::db>                 fb::db::_ist;

fb::db::db() : 
    _context(nullptr)
{
    auto& config = fb::config::get();
    auto& databases = config["database"];

    for(int i = 0; i < databases.size(); i++)
    {
        this->_pools.push_back(std::make_unique<fb::db::pool>(SIZE));
    };
}

fb::db::~db()
{ }

void fb::db::bind(boost::asio::io_context& context)
{
    get()._context = &context;
}

uint64_t fb::db::hash(const char* name)
{
    if(name == nullptr)
        return 0;

    auto hash = uint64_t(0);
    auto length = strlen(name);
    for(int i = 0; i < length; i++)
    {
        hash = 31 * hash + name[i];
    }

    return hash;
}

uint8_t fb::db::index(const char* name)
{
    if(name == nullptr)
        return 0;

    auto size = this->_pools.size();
    if(size == 1)
        return 0;

    return (uint8_t)(hash(name) % size - 1) + 1;
}

fb::db::pool* fb::db::connections(const char* name)
{
    auto index = fb::db::index(name);
    return this->_pools[index].get();
}

fb::db::connection&& fb::db::get(const char* name)
{
    std::lock_guard<std::mutex> mg(this->_mutex);

    auto& c = fb::console::get();
    auto connections = fb::db::connections(name);
    while(connections->empty())
    {
        c.puts("All connections are used.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    auto index = fb::db::index(name);
    auto&& connection = std::move(connections->front());
    connections->pop_front();

    if(connection == nullptr)
    {
        auto& config = fb::config::get();
        auto option = daotk::mysql::connect_options();
        option.server = config["database"][index]["ip"].asString();
        option.port = config["database"][index]["port"].asInt();
        option.username = config["database"][index]["uid"].asString();
        option.password = config["database"][index]["pwd"].asString();
        option.dbname = config["database"][index]["name"].asString();
        option.autoreconnect = true;

        connection.reset(new daotk::mysql::connection(option));
    }

    if(connection->is_open())
        connection->set_server_option(MYSQL_OPTION_MULTI_STATEMENTS_ON);

    return std::move(connection);
}

bool fb::db::release(const char* name, fb::db::connection& connection)
{
    std::lock_guard<std::mutex> mg(this->_mutex);

    auto connections = fb::db::connections(name);

    if(connections == nullptr)
        return false;

    try
    {
        connections->push_back(std::move(connection));
        return true;
    }
    catch(std::exception&)
    {
        return false;
    }
}

void fb::db::_exec(const char* name, const std::string& sql)
{
    auto&& connection = db::get(name);
    try
    {
        connection->exec(sql);
    }
    catch(std::exception& e)
    {
        auto& c = console::get();
        c.puts(e.what());
    }

    this->release(name, connection);
}

void fb::db::_query(const char* name, const std::string& sql, const std::function<void(daotk::mysql::connection&, daotk::mysql::result&)>& fn)
{
    auto&& connection = db::get(name);
    auto   result = connection->query(sql);

    try
    {
        boost::asio::dispatch
        (
            *_context, 
            [this, connection = std::move(connection), name = std::string(name), &fn, result = std::move(result)] () mutable
            { 
                fn(*connection, result); 
                this->release(name.c_str(), connection);
            }
        );
    }
    catch(std::exception& e)
    {
        console::get().puts(e.what());
    }
}

void fb::db::_mquery(const char* name, const std::string& sql, const std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)>& fn)
{
    auto&& connection = db::get(name);
    auto   result = connection->mquery(sql);

    try
    {
        boost::asio::dispatch
        (
            *_context, 
            [this, connection = std::move(connection), name = std::string(name), fn, result = std::move(result)] () mutable
            { 
                fn(*connection, result); 
                this->release(name.c_str(), connection);
            }
        );
    }
    catch(std::exception& e)
    {
        console::get().puts(e.what());
    }
}

fb::db& fb::db::get()
{
    static std::once_flag flag;
    std::call_once(flag, [] () { _ist.reset(new fb::db()); });

    return *_ist;
}

bool fb::db::query(const char* name, const std::vector<std::string>& queries)
{
    auto& ist = get();
    if(ist._context == nullptr)
        return false;

    std::stringstream sstream;
    for(int i = 0; i < queries.size(); i++)
    {
        if(i > 0)
            sstream << "; ";

        sstream << queries[i];
    }

    fb::async::launch
    (
        [&ist, name = std::string(name), query = std::string(sstream.str())]()
        {
            ist._exec(name.c_str(), query.c_str());
        }
    );

    return true;
}

bool fb::db::query(const char* name, const std::function<void()>& fn, const std::vector<std::string>& queries)
{
    auto& ist = get();
    if(ist._context == nullptr)
        return false;

    std::stringstream sstream;
    for(int i = 0; i < queries.size(); i++)
    {
        if(i > 0)
            sstream << "; ";

        sstream << queries[i];
    }

    fb::async::launch
    (
        [&ist, name = std::string(name), query = std::string(sstream.str()), fn]()
        {
            ist._exec(name.c_str(), query.c_str());
            fn();
        }
    );

    return true;
}
