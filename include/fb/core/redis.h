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

namespace fb {

class redis_stream_pool
{
private:
    std::queue<std::shared_ptr<cpp_redis::client>> _idle;
    std::set<std::shared_ptr<cpp_redis::client>> _busy;
    std::mutex _mutex;

public:
    redis_stream_pool() = default;
    ~redis_stream_pool() = default;

public:
    std::shared_ptr<cpp_redis::client> get()
    {
        auto _ = std::lock_guard(_mutex);

        if (this->_idle.empty() == false)
        {
            auto& client = this->_idle.front();
            this->_busy.insert(client);
            this->_idle.pop();

            return client;
        }
        else
        {
            auto client = std::make_shared< cpp_redis::client>();
            client->connect();
            this->_busy.insert(client);

            return client;
        }
    }

    void release(std::shared_ptr<cpp_redis::client>& client)
    {
        auto _ = std::lock_guard(_mutex);

        this->_idle.push(client);
        this->_busy.erase(client);
    }
};


class redis
{
private:
    icontext& _owner;
    redis_stream_pool _pool;

public:
	redis(icontext& owner) : _owner(owner)
	{ }
	~redis() = default;

private:
    template <typename T>
    void try_lock(const std::string& key, fb::awaitable<T>& awaitable, const std::function<T()>& fn, const std::string& uuid, std::shared_ptr<cpp_redis::subscriber> subscriber, fb::thread* thread)
    {
        auto conn = std::make_shared< cpp_redis::client>();
        conn->connect();
        auto script =
            "local success = redis.call('setnx', KEYS[1], 'lock')\n"
            "if success == 1 then\n"
            "    redis.call('expire', KEYS[1], 5)\n"
            "end\n"
            "return success";
        conn->eval(script, 1, { key }, {}, [&, this, conn, key, uuid, subscriber, thread](cpp_redis::reply& v) mutable
            {
                auto success = v.as_integer() == 1;
                if (success)
                {
                    auto action = [this](auto& awaitable, auto& fn, const std::string key, const std::string uuid, auto conn)
                    {
                        auto result = fn();

                        auto new_conn = std::make_shared< cpp_redis::client>();
                        new_conn->connect();

                        new_conn->del({ key });
                        new_conn->publish(key, uuid, [&, this, conn, key, result](cpp_redis::reply& reply) mutable
                            {
                                //this->_pool.release(conn);
                            });

                        new_conn->commit();

                        awaitable.result = &result;
                        awaitable.handler.resume();
                    };

                    if (thread != nullptr)
                    {
                        thread->dispatch([&action, &awaitable, &fn, key, uuid, conn](uint8_t) { action(awaitable, fn, key, uuid, conn); });
                    }
                    else
                    {
                        boost::thread{ [&action, &awaitable, &fn, key, uuid, conn]() { action(awaitable, fn, key, uuid, conn); } };
                    }

                    fb::logger::debug("리소스 점유에 성공");
                }
                else
                {
                    fb::logger::debug("리소스 점유에 실패하여 대기");
                }
            });
        conn->commit();
    }


public:
	template <typename T>
    fb::awaitable<T> sync(const std::string& key, const std::function<T()>& fn)
    {
        auto callback = [this, key, &fn](auto& awaitable)
        {
            auto thread = this->_owner.current_thread();
            auto uuid = boost::uuids::to_string(boost::uuids::random_generator()());
            auto subscriber = std::make_shared<cpp_redis::subscriber>();
            subscriber->connect();
            subscriber->subscribe(key, [&, this, key, uuid, subscriber, thread](const std::string& chan, const std::string& msg)
                {
                    if (chan != key)
                        return;

                    if (msg != uuid)
                        this->try_lock(key, awaitable, fn, uuid, subscriber, thread);
                });
            subscriber->commit();

            this->try_lock(key, awaitable, fn, uuid, subscriber, thread);
        };

        return fb::awaitable<T>(callback);
    }
};

}

#endif