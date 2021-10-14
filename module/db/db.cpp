#include "db.h"

std::unique_ptr<fb::db>                 fb::db::_ist;

fb::db::db() : 
    _context(nullptr)
{
    auto& config = fb::config::get();
    auto& databases = config["database"];

    for(int i = 0; i < databases.size(); i++)
    {
        auto connections = new fb::db::pool(SIZE, nullptr);
        this->_pools.push_back(connections);
    };
}

fb::db::~db()
{
    std::lock_guard<std::mutex> mg(this->_mutex);

    auto size = this->_pools.size();
    for(int i = 0; i < size; i++)
    {
        auto connections = this->_pools[i];
        for(auto connection : *connections)
            delete connection;

        delete connections;
    }
    this->_pools.clear();
}

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
    return this->_pools[index];
}

daotk::mysql::connection& fb::db::get(const char* name)
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
    }

    if(connection->is_open())
        connection->set_server_option(MYSQL_OPTION_MULTI_STATEMENTS_ON);

    return *connection;
}

bool fb::db::release(const char* name, daotk::mysql::connection& connection)
{
    std::lock_guard<std::mutex> mg(this->_mutex);

    auto connections = fb::db::connections(name);

    if(connections == nullptr)
        return false;

    try
    {
        connections->push_back(&connection);
        return true;
    }
    catch(std::exception&)
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

    this->release(name, connection);
}

void fb::db::_query(const char* name, const std::string& sql, const const std::function<void(daotk::mysql::connection&, daotk::mysql::result&)>& fn)
{
    auto connection = &db::get(name);
    auto name_c = new std::string(name);

    try
    {
        auto result = new daotk::mysql::result(connection->query(sql));

        boost::asio::dispatch
        (
            *_context, 
            [this, connection, name_c, &fn, result] () 
            { 
                fn(*connection, *result); 
                delete result;
                this->release(name_c->c_str(), *connection);
                delete name_c;
            }
        );
    }
    catch(std::exception& e)
    {
        this->release(name, *connection);
        console::get().puts(e.what());

        delete name_c;
    }
}

void fb::db::_mquery(const char* name, const std::string& sql, const const std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)>& fn)
{
    auto connection = &db::get(name);
    auto name_c = new std::string(name);

    try
    {
        auto results = new std::vector<daotk::mysql::result>(connection->mquery(sql));

        boost::asio::dispatch
        (
            *_context, 
            [this, connection, name_c, fn, results] () 
            { 
                fn(*connection, *results); 
                delete results;
                this->release(name_c->c_str(), *connection);
                delete name_c;
            }
        );
    }
    catch(std::exception& e)
    {
        this->release(name, *connection);
        console::get().puts(e.what());

        delete name_c;
    }
}

fb::db& fb::db::get()
{
    if(_ist.get() == nullptr)
        _ist.reset(new fb::db());

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

    auto _name = new std::string(name);
    auto _qry = new std::string(sstream.str());

    fb::async::launch
    (
        [&ist, _name, _qry]()
        {
            ist._exec(_name->c_str(), _qry->c_str());
            delete _name;
            delete _qry;
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

    auto _name = new std::string(name);
    auto _qry = new std::string(sstream.str());

    fb::async::launch
    (
        [&ist, _name, _qry, fn]()
        {
            ist._exec(_name->c_str(), _qry->c_str());
            fn();
            delete _name;
            delete _qry;
        }
    );

    return true;
}
