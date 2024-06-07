#ifndef __REDIS_H__
#define __REDIS_H__

#include <functional>
#include <queue>
#include <set>
#include <map>
#include <cpp_redis/cpp_redis>
#include <fb/core/coroutine.h>
#include <fb/core/abstract.h>
#include <fb/core/logger.h>
#include <fb/core/mst.h>
#include <boost/thread.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace fb {

template <typename T>
class redis_pool
{
private:
    const std::string               _ip;
    uint16_t                        _port;

private:
    std::queue<std::shared_ptr<T>>  _idle;
    std::set<std::shared_ptr<T>>    _busy;
    std::mutex                      _mutex;

public:
    redis_pool(const std::string& ip, uint16_t port) : _ip(ip), _port(port)
    { }
    ~redis_pool() = default;

public:
    std::shared_ptr<T> get()
    {
        auto _ = std::lock_guard(this->_mutex);

        if (this->_idle.empty() == false)
        {
            auto& item = this->_idle.front();
            this->_busy.insert(item);
            this->_idle.pop();

            return item;
        }
        else
        {
            auto item = std::make_shared<T>();
            item->connect(this->_ip, this->_port, [](const std::string& host, std::size_t port, auto status)
            {
                // CHECK CONNECTING STATE
            });

            while (item->is_connected() == false)
            {
                std::this_thread::sleep_for(10ms);
            }
            this->_busy.insert(item);
            return item;
        }
    }

    void release(std::shared_ptr<T>& item)
    {
        auto _ = std::lock_guard(_mutex);

        this->_idle.push(item);
        this->_busy.erase(item);
    }
};

class redis
{
private:
    icontext&                           _owner;
    uint8_t                             _timeout;
    std::map<std::string, std::string>  _scripts;
    std::shared_ptr<fb::mst>            _root;
    std::set<std::string>               _routes;

public:
    redis_pool<cpp_redis::subscriber>   subs;
    redis_pool<cpp_redis::client>       conn;

public:
    redis(icontext& owner, const std::string& ip = "127.0.0.1", uint16_t port = 6379, uint8_t timeout = 5) : _owner(owner), subs(ip, port), conn(ip, port), _timeout(timeout), _root(std::make_shared<fb::mst>())
    {
        auto conn = this->conn.get();
        auto script =
            "local success = redis.call('setnx', KEYS[1], 'lock')\n"
            "if success == 1 then\n"
            "    redis.call('expire', KEYS[1], ARGV[1])\n"
            "end\n"
            "return success";

        conn->script_load(script, [this, conn](auto& reply) mutable
        {
            auto hash = reply.as_string();
            _scripts.insert({"lock", hash});
            this->conn.release(conn);
        });
        conn->sync_commit();
    }
    ~redis() = default;

private:
    fb::task<void>      handle_locked(fb::awaiter<int>& awaiter, const std::function<fb::task<int>(std::shared_ptr<fb::mst>&)>& fn, std::shared_ptr<fb::mst> current, const std::string key, const std::string uuid, std::shared_ptr<cpp_redis::client> conn, std::shared_ptr<cpp_redis::subscriber> subs);
    void                try_lock(const std::string& key, fb::awaiter<int>& awaiter, const std::function<fb::task<int>(std::shared_ptr<fb::mst>&)>& fn, const std::string& uuid, std::shared_ptr<cpp_redis::client> conn, std::shared_ptr<cpp_redis::subscriber> subs, fb::thread* thread, std::shared_ptr<fb::mst>& trans);

public:
    fb::awaiter<int>    sync(const std::string& key, const std::function<fb::task<int>(std::shared_ptr<fb::mst>&)>& fn, std::shared_ptr<fb::mst>& trans);
    fb::awaiter<int>    sync(const std::string& key, const std::function<fb::task<int>(std::shared_ptr<fb::mst>&)>& fn);
};

}

#endif