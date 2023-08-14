#include <fb/core/db.h>

std::unique_ptr<fb::db>                 fb::db::_ist;

fb::db::db() : 
    _context(nullptr)
{
    auto& config    = fb::config::get();
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

uint64_t fb::db::hash(const std::string& name)
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

uint8_t fb::db::index(const std::string& name)
{
    if(name.empty())
        return 0;

    auto size = this->_pools.size();
    if(size == 1)
        return 0;

    return (uint8_t)(hash(name) % size - 1) + 1;
}

fb::db::pool* fb::db::connections(const std::string& name)
{
    auto index = fb::db::index(name);
    return this->_pools[index].get();
}

fb::db::connection fb::db::get(const std::string& name)
{
this->_mutex.lock();
    auto& c                  = fb::console::get();
    auto  connections        = fb::db::connections(name);

    auto  connection         = fb::db::connection(nullptr);
    auto  busy               = false;
    while(true)
    {
        if (connections->empty() == false)
            break;
        
this->_mutex.unlock();
        busy = true;
        std::this_thread::sleep_for(100ms);
this->_mutex.lock();
    }

    auto index             = fb::db::index(name);
    connection             = std::move(connections->front());
    if (busy)
    {
        c.puts("db reconnect success");
        busy = false;
    }

    connections->pop_front();
    this->_mutex.unlock();

    if(connection == nullptr)
    {
        auto& config         = fb::config::get();
        auto  option         = daotk::mysql::connect_options();
        option.server        = config["database"][index]["ip"].asString();
        option.port          = config["database"][index]["port"].asInt();
        option.username      = config["database"][index]["uid"].asString();
        option.password      = config["database"][index]["pwd"].asString();
        option.dbname        = config["database"][index]["name"].asString();
        option.autoreconnect = true;

        connection.reset(new daotk::mysql::connection(option));
        while (connection->is_open() == false)
        {
            busy = true;
            std::this_thread::sleep_for(100ms);
            connection->open(option);
        }

        if (busy)
        {
            c.puts("db re-open success");
            busy = false;
        }
    }

    connection->set_server_option(MYSQL_OPTION_MULTI_STATEMENTS_ON);
    return connection;
}

bool fb::db::release(const std::string& name, fb::db::connection& connection)
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

void fb::db::_exec(const std::string& name, const std::string& sql)
{
    auto connection = db::get(name);
    try
    {
        connection->mquery(sql);
    }
    catch(std::exception& e)
    {
        auto& c = console::get();
        c.puts(e.what());
    }

    this->release(name, connection);
}

void fb::db::_query(const std::string& name, const std::string& sql, const std::function<void(daotk::mysql::connection&, daotk::mysql::result&)>& fn)
{
    auto connection = db::get(name);
    auto result = connection->query(sql);

    try
    {
        fn(*connection, result); 
        this->release(name, connection);
    }
    catch(std::exception& e)
    {
        console::get().puts(e.what());
    }
}

void fb::db::_mquery(const std::string& name, const std::string& sql, const std::function<void(daotk::mysql::connection&, std::vector<daotk::mysql::result>&)>& fn)
{
    auto connection = db::get(name);
    auto result = connection->mquery(sql);

    try
    {
        fn(*connection, result); 
        this->release(name, connection);
    }
    catch(std::exception& e)
    {
        console::get().puts(e.what());
    }
}

fb::db& fb::db::get()
{
    static std::once_flag flag;
    std::call_once(flag, [] { _ist.reset(new fb::db()); });

    return *_ist;
}

void fb::db::query(const std::string& name, const std::vector<std::string>& queries)
{
    auto& ist = get();
    if(ist._context == nullptr)
        return;

    std::stringstream sstream;
    for (auto& query : queries)
    {
        if (query.empty())
            continue;

        sstream << query << ";";
    }

    ist._exec(name, sstream.str());
}

void fb::db::async_query(const std::string& name, const std::vector<std::string>& queries)
{
    fb::async::launch([name, queries]
    {
        query(name, queries);
    });
}

void fb::db::query(const std::string& name, const std::function<void()>& callback, const std::vector<std::string>& queries)
{
    auto& ist = get();
    if(ist._context == nullptr)
        return;

    std::stringstream sstream;
    for (auto& query : queries)
    {
        if (query.empty())
            continue;

        sstream << query << ";";
    }

    ist._exec(name, sstream.str());
    callback();
}

void fb::db::async_query(const std::string& name, const std::function<void()>& callback, const std::vector<std::string>& queries)
{
    fb::async::launch([name, callback, queries]
    {
        query(name, callback, queries);
    });
}

auto fb::db::co_query(const std::string& name, const std::vector<std::string>& queries)
{
    auto await_callback = [name, queries](fb::awaitable<void>& awaitable)
    {
        auto db_callback = [&awaitable]
        {
            awaitable.handler.resume();
        };

        async_query(name, db_callback, queries);
    };

    return fb::awaitable<void>(await_callback);
}

void fb::db::exec(const std::string& name, const std::string& sql)
{
    auto& ist = get();
    if (ist._context == nullptr)
        return;

    ist._exec(name, sql);
}

void fb::db::async_exec(const std::string& name, const std::string& sql)
{
    fb::async::launch([name, sql]
        {
            exec(name, sql);
        });
}