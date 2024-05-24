#ifndef	__REDIS_H__
#define	__REDIS_H__

#include <functional>
#include <queue>
#include <set>
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
class pool_pairs
{
private:
    std::queue<std::shared_ptr<T>>  _idle;
    std::set<std::shared_ptr<T>>    _busy;
    std::mutex                      _mutex;

public:
    pool_pairs() = default;
    ~pool_pairs() = default;

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
            auto item = std::make_shared< T>();
            item->connect();

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

class redis_stream_pool
{
public:
    pool_pairs<cpp_redis::subscriber>   subs;
    pool_pairs<cpp_redis::client>       conn;
};


class redis
{
private:
    icontext&                           _owner;

public:
    redis_stream_pool                   pool;

public:
	redis(icontext& owner) : _owner(owner)
	{ }
	~redis() = default;

private:
    template <typename T>
    void try_lock(const std::string& key, fb::awaitable<T>& awaitable, const std::function<T()>& fn, const std::string& uuid, std::shared_ptr<cpp_redis::client> conn, std::shared_ptr<cpp_redis::subscriber> subs, fb::thread* thread)
    {
        auto script =
        "local success = redis.call('setnx', KEYS[1], 'lock')\n"
        "if success == 1 then\n"
        "    redis.call('expire', KEYS[1], 5)\n"
        "end\n"
        "return success\n";


    conn->eval(script, 1, { key }, {}, [this, conn, subs, key, uuid, thread, &awaitable, &fn](cpp_redis::reply& v) mutable
        {
            auto success = v.as_integer() == 1;
            if (success)
            {
                auto handler = [this, &fn, conn, subs, key, uuid, &awaitable](uint8_t)
                {
                    auto result = fn();
                    awaitable.result = &result;
                    awaitable.handler.resume();

                    conn->del({ key }, [this, conn, subs, key](auto& reply)  mutable
                    {
                        subs->unsubscribe(key);
                        subs->commit();
                        this->pool.subs.release(subs);
                    });
                    conn->publish(key, uuid, [this, conn](auto& reply)  mutable
                    {
                        this->pool.conn.release(conn);
                    });
                    conn->commit();
                };

                if (thread != nullptr)
                    thread->dispatch(handler);
                else
                    handler(-1);
            }
        });
    conn->commit();
    }


public:
    template <typename T>
    fb::awaitable<T> sync(const std::string& key, const std::function<T()>& fn)
    {
        return fb::awaitable<T>([this, key, &fn](auto& awaitable) mutable
    {
        auto conn = this->pool.conn.get();
        auto subs = this->pool.subs.get();
        auto thread = this->_owner.current_thread();
        auto uuid = boost::uuids::to_string(boost::uuids::random_generator()());
        subs->subscribe(key, [&, this, key, uuid, conn, subs, thread](const std::string& chan, const std::string& msg) mutable
        {
            if (chan != key)
                return;

            if (msg != uuid)
                this->try_lock(key, awaitable, fn, uuid, conn, subs, thread);
        });
        subs->commit();
        this->try_lock(key, awaitable, fn, uuid, conn, subs, thread);
    });
    }
};

}

#endif