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

public:
    redis_pool<cpp_redis::subscriber>   subs;
    redis_pool<cpp_redis::client>       conn;

public:
    redis(icontext& owner, const std::string& ip = "127.0.0.1", uint16_t port = 6379, uint8_t timeout = 5) : _owner(owner), subs(ip, port), conn(ip, port), _timeout(timeout)
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
    template <typename T>
    void try_lock(const std::string& key, fb::awaiter<T>& awaiter, const std::function<fb::task<T>()>& fn, const std::string& uuid, std::shared_ptr<cpp_redis::client> conn, std::shared_ptr<cpp_redis::subscriber> subs, fb::thread* thread)
    {
        conn->evalsha(this->_scripts["lock"], 1, { key }, { std::to_string(this->_timeout) }, [this, conn, subs, key, uuid, thread, &awaiter, &fn](cpp_redis::reply& v) mutable
        {
            auto success = v.as_integer() == 1;
            if (success == false)
                return;

            auto handler = [this, &fn, conn, subs, key, uuid](auto& awaiter) mutable -> fb::task<void>
            {
                try
                {
                    auto result = co_await fn();
                    awaiter.result = &result;
                }
                catch (std::exception& e)
                {
                    awaiter.error = std::make_unique<std::runtime_error>(e.what());
                }
                awaiter.handler.resume();

                conn->del({ key }, [this, conn, subs, key](auto& reply) mutable
                {
                    subs->unsubscribe(key);
                    subs->commit();
                    this->subs.release(subs);
                });
                conn->publish(key, uuid, [this, conn](auto& reply) mutable
                {
                    this->conn.release(conn);
                });
                conn->commit();
            };
            
            if (thread != nullptr)
            {
                thread->dispatch([this, handler, &awaiter](uint8_t) mutable { handler(awaiter); });
            }
            else
            {
                handler(awaiter);
            }
        });
        conn->commit();
    }


public:
    template <typename T>
    fb::awaiter<T> sync(const std::string& key, const std::function<fb::task<T>()>& fn)
    {
        return fb::awaiter<T>([this, key, &fn](auto& awaiter) mutable
        {
            try
            {
                auto conn = this->conn.get();
                auto subs = this->subs.get();
                auto thread = this->_owner.current_thread();
                auto uuid = boost::uuids::to_string(boost::uuids::random_generator()());
                subs->subscribe(key, [&, this, key, uuid, conn, subs, thread](const std::string& chan, const std::string& msg) mutable
                {
                    if (chan != key)
                        return;

                    if (msg != uuid)
                        this->try_lock(key, awaiter, fn, uuid, conn, subs, thread);
                });
                subs->commit();
                this->try_lock(key, awaiter, fn, uuid, conn, subs, thread);
            }
            catch(std::exception& e)
            {
                awaiter.error = std::make_unique<std::runtime_error>(e.what());
                awaiter.handler.resume();
            }
        });
    }
};

}

#endif