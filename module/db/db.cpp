#include "db.h"

boost::asio::io_context*                fb::db::_context = nullptr;
uint32_t                                fb::db::_index = 0;
std::deque<daotk::mysql::connection*>*  fb::db::_connections = nullptr;
std::deque<daotk::mysql::connection*>*  fb::db::_usings = nullptr;
std::mutex                              fb::db::_mutex;

void fb::db::bind(boost::asio::io_context& context)
{
    _context = &context;
}

daotk::mysql::connection& fb::db::get()
{
_mutex.lock();
    if(_connections == nullptr)
        _connections = new std::deque<daotk::mysql::connection*>(SIZE, nullptr);

    if(_usings == nullptr)
        _usings = new std::deque<daotk::mysql::connection*>();

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
    }

    _usings->push_back(connection);
_mutex.unlock();

    return *connection;
}

bool fb::db::release(daotk::mysql::connection& connection)
{
    if(_connections == nullptr)
        return false;

    if(_usings == nullptr)
        return false;

    try
    {
_mutex.lock();
        auto found = std::find(_usings->begin(), _usings->end(), &connection);
        if(found == _usings->end())
            throw std::exception();

        _usings->erase(found);
        _connections->push_back(&connection);
_mutex.unlock();
        return true;
    }
    catch(std::exception& e)
    {
_mutex.unlock();
        return false;
    }
}

void fb::db::release()
{
_mutex.lock();
    if(_connections != nullptr)
    {
        for(auto connection : *_connections)
            delete connection;

        delete _connections;
        _connections = nullptr;
    }

    if(_usings != nullptr)
    {
        for(auto connection : *_usings)
            delete connection;

        delete _usings;
        _usings = nullptr;
    }
_mutex.unlock();
}