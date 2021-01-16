#include "db.h"

boost::asio::io_context*                fb::db::_context = nullptr;
uint32_t                                fb::db::_index = 0;
std::deque<daotk::mysql::connection*>*  fb::db::_connections = nullptr;
std::mutex                              fb::db::_mutex;

void fb::db::bind(boost::asio::io_context& context)
{
    _context = &context;
}

daotk::mysql::connection& fb::db::get()
{
    std::lock_guard<std::mutex> mg(_mutex);

    if(_connections == nullptr)
        _connections = new std::deque<daotk::mysql::connection*>(SIZE, nullptr);

    auto& c = fb::console::get();
    while(_connections->empty())
    {
        c.puts("All connections are used.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    auto connection = _connections->front();
    _connections->pop_front();

    if(connection == nullptr)
    {
        auto& config = fb::config::get();
        connection = new daotk::mysql::connection
        (
            config["database"]["ip"].asString(), 
            config["database"]["uid"].asString(), 
            config["database"]["pwd"].asString(), 
            config["database"]["name"].asString()
        );
        connection->set_server_option(MYSQL_OPTION_MULTI_STATEMENTS_ON);
    }

    if(connection->is_open() == false)
    {
        delete connection;
        
        auto& config = fb::config::get();
        connection = new daotk::mysql::connection
        (
            config["database"]["ip"].asString(), 
            config["database"]["uid"].asString(), 
            config["database"]["pwd"].asString(), 
            config["database"]["name"].asString()
        );
        connection->set_server_option(MYSQL_OPTION_MULTI_STATEMENTS_ON);
    }

    return *connection;
}

bool fb::db::release(daotk::mysql::connection& connection)
{

    std::lock_guard<std::mutex> mg(_mutex);

    if(_connections == nullptr)
        return false;

    try
    {
        _connections->push_back(&connection);
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

void fb::db::_exec(const std::string& sql)
{
    auto& connection = db::get();
    try
    {
        connection.exec(sql);
    }
    catch(std::exception& e)
    {
        auto& c = console::get();
        c.puts(e.what());
    }

    db::release(connection);
}

void fb::db::_query(const std::string& sql, const std::function<void(daotk::mysql::connection&, daotk::mysql::result&)>& callback)
{
    auto connection = &db::get();
    try
    {
        auto result = new daotk::mysql::result(connection->query(sql));

        boost::asio::dispatch
        (
            *_context, 
            [connection, callback, result] () 
            { 
                callback(*connection, *result); 
                delete result;
                db::release(*connection);
            }
        );
    }
    catch(std::exception& e)
    {
        db::release(*connection);
        console::get().puts(e.what());
    }
}

void fb::db::_mquery(const std::string& sql, const std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)>& callback)
{
    auto connection = &db::get();
    try
    {
        auto results = new std::vector<daotk::mysql::result>(connection->mquery(sql));

        boost::asio::dispatch
        (
            *_context, 
            [connection, callback, results] () 
            { 
                callback(*connection, *results); 
                delete results;
                db::release(*connection);
            }
        );
    }
    catch(std::exception& e)
    {
        db::release(*connection);
        console::get().puts(e.what());
    }
}

void fb::db::close()
{
    std::lock_guard<std::mutex> mg(_mutex);

    if(_connections != nullptr)
    {
        for(auto connection : *_connections)
            delete connection;

        delete _connections;
        _connections = nullptr;
    }
}

bool fb::db::mquery(std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)> callback, const std::vector<std::string>& queries)
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
        std::bind(&db::_mquery, std::placeholders::_1, std::placeholders::_2),
        sstream.str(),
        callback
    );

    return true;
}