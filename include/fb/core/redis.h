#ifndef __REDIS_H__
#define __REDIS_H__

#include <functional>
#include <set>
#include <cpp_redis/cpp_redis>
#include <fb/core/abstract.h>
#include <fb/core/coroutine.h>
#include <fb/core/concurrent.h>
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

class redis : public fb::concurrent
{
private:
    icontext&                           _owner;
    uint8_t                             _timeout;
    std::map<std::string, std::string>  _scripts;
    std::set<std::string>               _routes;

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
    fb::task<void> handle_locked(fb::awaiter<T>& awaiter, const std::function<fb::task<T>(fb::dead_lock_detector&)>& fn, fb::dead_lock_detector& current, const std::string key, const std::string uuid, std::shared_ptr<cpp_redis::client> conn, std::shared_ptr<cpp_redis::subscriber> subs)
    {
        try
        {
            awaiter.result(co_await fn(current));
            concurrent::add(current);
        }
        catch (std::exception& e)
        {
            awaiter.error(e);
        }

        conn->del({ key }, [this, subs, key](auto& reply) mutable
        {
            subs->unsubscribe(key);
            subs->commit();
            this->subs.release(subs);
        });
        conn->publish(key, uuid, [this, key, conn, &awaiter](auto& reply) mutable
        {
            this->conn.release(conn);
            awaiter.resume();
        });
        conn->commit();
    }

    template <typename T>
    fb::task<void> handle_locked(fb::awaiter<T>& awaiter, const std::function<fb::task<T>(void)>& fn, const std::string key, const std::string uuid, std::shared_ptr<cpp_redis::client> conn)
    {
        try
        {
            awaiter.result(co_await fn());
        }
        catch (std::exception& e)
        {
            awaiter.error(e);
        }

        conn->del({ key });
        conn->publish(key, uuid, [this, key, conn, &awaiter](auto& reply) mutable
        {
            this->conn.release(conn);
            awaiter.resume();
        });
        conn->commit();
    }

    template <typename T>
    bool lock(const std::string& key, fb::awaiter<T>& awaiter, const std::function<fb::task<T>(fb::dead_lock_detector&)>& fn, const std::string& uuid, std::shared_ptr<cpp_redis::client> conn, std::shared_ptr<cpp_redis::subscriber> subs, fb::thread* thread, fb::dead_lock_detector& trans)
    {
        auto& current = static_cast<fb::mst<std::string>&>(trans).add<fb::dead_lock_detector>(key);

        try
        {
            concurrent::assert_dead_lock(current);
        }
        catch (std::exception& e)
        {
            awaiter.resume(e);
            return false;
        }

        conn->evalsha(this->_scripts["lock"], 1, { key }, { std::to_string(this->_timeout) }, [this, conn, subs, key, uuid, thread, &awaiter, &fn, &current](cpp_redis::reply& v) mutable
        {
            auto success = v.as_integer() == 1;
            if (success == false)
                return;

            if (thread != nullptr)
            {
                thread->dispatch([this, &awaiter, &fn, &current, key, uuid, conn, subs]() mutable
                {
                    this->handle_locked(awaiter, fn, current, key, uuid, conn, subs);
                });
            }
            else
            {
                this->handle_locked(awaiter, fn, current, key, uuid, conn, subs);
            }
        });
        conn->commit();
        return true;
    }

    template <typename T>
    void try_lock(const std::string& key, fb::awaiter<T>& awaiter, const std::function<fb::task<T>(void)>& fn, const std::string& uuid, std::shared_ptr<cpp_redis::client> conn, fb::thread* thread)
    {
        conn->evalsha(this->_scripts["lock"], 1, { key }, { std::to_string(this->_timeout) }, [this, conn, key, uuid, thread, &awaiter, &fn](cpp_redis::reply& v) mutable
        {
            auto success = v.as_integer() == 1;
            if (success == false)
            {
                awaiter.resume(fb::lock_error());
            }
            else if (thread != nullptr)
            {
                thread->dispatch([this, &awaiter, &fn, key, uuid, conn]() mutable 
                {
                    this->handle_locked(awaiter, fn, key, uuid, conn);
                });
            }
            else
            {
                this->handle_locked(awaiter, fn, key, uuid, conn);
            }
        });
        conn->commit();
    }

public:
    template <typename T>
    fb::awaiter<T> sync(const std::string& key, const std::function<fb::task<T>(fb::dead_lock_detector&)>& fn, fb::dead_lock_detector& trans)
    {
        return fb::awaiter<T>([this, key, &fn, &trans](auto& awaiter) mutable
        {
            auto conn = this->conn.get();
            auto subs = this->subs.get();
            auto thread = this->_owner.current_thread();
            auto uuid = boost::uuids::to_string(boost::uuids::random_generator()());
            subs->subscribe(key, [this, key, uuid, conn, subs, &awaiter, &fn, thread, &trans](const std::string& chan, const std::string& msg) mutable
            {
                if (chan != key)
                    return;

                if (msg != uuid)
                    this->lock(key, awaiter, fn, uuid, conn, subs, thread, trans);
            });

            if (this->lock(key, awaiter, fn, uuid, conn, subs, thread, trans))
                subs->commit();
        });
    }

    template <typename T>
    fb::awaiter<T> sync(const std::string& key, const std::function<fb::task<T>(fb::dead_lock_detector&)>& fn)
    {
        return this->sync(key, fn, this->root);
    }

    template <typename T>
    fb::awaiter<T> try_sync(const std::string& key, const std::function<fb::task<T>(void)>& fn)
    {
        return fb::awaiter<T>([this, key, &fn](auto& awaiter) mutable
        {
            auto conn = this->conn.get();
            auto thread = this->_owner.current_thread();
            auto uuid = boost::uuids::to_string(boost::uuids::random_generator()());
            this->try_lock(key, awaiter, fn, uuid, conn, thread);
        });
    }
};

}

#endif