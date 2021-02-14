#include "db.h"

boost::asio::io_context*                fb::db::_context = nullptr;
uint32_t                                fb::db::_index = 0;
fb::db::pools                           fb::db::_pools;
std::mutex                              fb::db::_mutex;

void fb::db::bind(boost::asio::io_context& context)
{
    _context = &context;
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

    auto size = _pools.size();
    if(size == 1)
        return 0;

    return (uint8_t)(hash(name) % size - 1) + 1;
}

fb::db::pool* fb::db::connections(const char* name)
{
    auto index = fb::db::index(name);
    auto connections = _pools[index];
    if(connections == nullptr)
        connections = new std::deque<daotk::mysql::connection*>(SIZE, nullptr);

    return connections;
}

daotk::mysql::connection& fb::db::get(const char* name)
{
    std::lock_guard<std::mutex> mg(_mutex);

    auto& c = fb::console::get();
    auto connections = fb::db::connections(name);
    while(connections->empty())
    {
        c.puts("All connections are used.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    auto index = fb::db::index(name);
    auto connection = connections->front();
    connections->pop_front();

    if(connection == nullptr)
    {
        auto& config = fb::config::get();
        connection = new daotk::mysql::connection
        (
            config["database"][index]["ip"].asString(), 
            config["database"][index]["uid"].asString(), 
            config["database"][index]["pwd"].asString(), 
            config["database"][index]["name"].asString()
        );
        connection->set_server_option(MYSQL_OPTION_MULTI_STATEMENTS_ON);
    }

    if(connection->is_open() == false)
    {
        delete connection;
        
        auto& config = fb::config::get();
        connection = new daotk::mysql::connection
        (
            config["database"][index]["ip"].asString(), 
            config["database"][index]["uid"].asString(), 
            config["database"][index]["pwd"].asString(), 
            config["database"][index]["name"].asString()
        );
        connection->set_server_option(MYSQL_OPTION_MULTI_STATEMENTS_ON);
    }

    return *connection;
}

bool fb::db::release(const char* name, daotk::mysql::connection& connection)
{
    std::lock_guard<std::mutex> mg(_mutex);

    auto connections = fb::db::connections(name);

    if(connections == nullptr)
        return false;

    try
    {
        connections->push_back(&connection);
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

void fb::db::_exec(const char* name, const std::string& sql)
{
    auto& connection = db::get(name);
    try
    {
        connection.exec(sql);
    }
    catch(std::exception& e)
    {
        auto& c = console::get();
        c.puts(e.what());
    }

    db::release(name, connection);
}

bool fb::db::query(const char* name, const std::vector<std::string>& queries)
{
    if(_context == nullptr)
        return false;

    std::stringstream sstream;
    for(int i = 0; i < queries.size(); i++)
    {
        if(i > 0)
            sstream << "; ";

        sstream << queries[i];
    }

    auto future = std::async
    (
        std::launch::async, 
        std::bind(&db::_exec, std::placeholders::_1, std::placeholders::_2),
        name,
        sstream.str()
    );

    return true;
}

void fb::db::_query(const char* name, const std::string& sql, const std::function<void(daotk::mysql::connection&, daotk::mysql::result&)>& callback)
{
    auto connection = &db::get(name);
    auto name_c = new std::string(name);

    try
    {
        auto result = new daotk::mysql::result(connection->query(sql));

        boost::asio::dispatch
        (
            *_context, 
            [connection, name_c, callback, result] () 
            { 
                callback(*connection, *result); 
                delete result;
                db::release(name_c->c_str(), *connection);
                delete name_c;
            }
        );
    }
    catch(std::exception& e)
    {
        db::release(name, *connection);
        console::get().puts(e.what());

        delete name_c;
    }
}

void fb::db::_mquery(const char* name, const std::string& sql, const std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)>& callback)
{
    auto connection = &db::get(name);
    auto name_c = new std::string(name);
    try
    {
        auto results = new std::vector<daotk::mysql::result>(connection->mquery(sql));

        boost::asio::dispatch
        (
            *_context, 
            [connection, name_c, callback, results] () 
            { 
                callback(*connection, *results); 
                delete results;
                db::release(name_c->c_str(), *connection);
                delete name_c;
            }
        );
    }
    catch(std::exception& e)
    {
        db::release(name, *connection);
        console::get().puts(e.what());

        delete name_c;
    }
}

void fb::db::init()
{
    auto& config = fb::config::get();
    auto databases = config["database"];

    for(int i = 0; i < databases.size(); i++)
    {
        auto connections = new fb::db::pool(SIZE, nullptr);
        _pools.push_back(connections);
    };
}

void fb::db::close()
{
    std::lock_guard<std::mutex> mg(_mutex);

    auto size = _pools.size();
    for(int i = 0; i < size; i++)
    {
        auto connections = _pools[i];
        for(auto connection : *connections)
            delete connection;

        delete connections;
    }
    _pools.clear();
}

bool fb::db::query(const char* name, std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)> callback, const std::vector<std::string>& queries)
{
    if(_context == nullptr)
        return false;

    std::stringstream sstream;
    for(int i = 0; i < queries.size(); i++)
    {
        if(i > 0)
            sstream << "; ";

        sstream << queries[i];
    }

    auto future = std::async
    (
        std::launch::async, 
        std::bind(&db::_mquery, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        name, 
        sstream.str(),
        callback
    );

    return true;
}
