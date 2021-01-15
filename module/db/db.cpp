#include "db.h"

daotk::mysql::connection**          fb::db::_connection_pool = nullptr;
boost::asio::io_context*            fb::db::_context = nullptr;
uint32_t                            fb::db::_index = 0;

void fb::db::bind(boost::asio::io_context& context)
{
    _context = &context;
}

daotk::mysql::connection& fb::db::get()
{
    auto& config = fb::config::get();
    if(_connection_pool == nullptr)
    {
        _connection_pool = new daotk::mysql::connection*[SIZE];
        for(int i = 0; i < SIZE; i++)
        {
            _connection_pool[i] = new daotk::mysql::connection
            (
                config["database"]["ip"].asString(), 
                config["database"]["uid"].asString(), 
                config["database"]["pwd"].asString(), 
                config["database"]["name"].asString()
            );
        }
    }

    auto current = _connection_pool[_index];
    if(current->is_open() == false)
    {
        delete _connection_pool[_index];
        current = _connection_pool[_index] = new daotk::mysql::connection
        (
            config["database"]["ip"].asString(), 
            config["database"]["uid"].asString(), 
            config["database"]["pwd"].asString(), 
            config["database"]["name"].asString()
        );
    }

    _index = (_index + 1) % SIZE;
    return *current;
}

void fb::db::release()
{
    if(_connection_pool == nullptr)
        return;

    for(int i = 0; i < SIZE; i++)
        delete _connection_pool[i];

    delete[] _connection_pool;
}