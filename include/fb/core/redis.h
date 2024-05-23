#ifndef	__REDIS_H__
#define	__REDIS_H__

#include <functional>
#include <queue>
#include <set>
#include <redis-cpp/stream.h>
#include <redis-cpp/execute.h>
#include <fb/core/coroutine.h>
#include <fb/core/abstract.h>
#include <fb/core/logger.h>

namespace fb {

class redis_stream_pool
{
private:
    std::queue<std::shared_ptr<std::iostream>> _idle;
    std::set<std::shared_ptr<std::iostream>> _busy;
    std::mutex _mutex;

public:
    redis_stream_pool() = default;
    ~redis_stream_pool() = default;

public:
    std::shared_ptr<std::iostream> get()
    {
        auto _ = std::lock_guard(_mutex);

        if (this->_idle.empty() == false)
        {
            auto& stream = this->_idle.front();
            this->_busy.insert(stream);
            this->_idle.pop();

            return stream;
        }
        else
        {
            auto stream = rediscpp::make_stream("localhost", "6379");
            this->_busy.insert(stream);

            return stream;
        }
    }

    void release(std::shared_ptr<std::iostream>& stream)
    {
        auto _ = std::lock_guard(_mutex);

        this->_idle.push(stream);
        this->_busy.erase(stream);
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


public:
	template <typename T>
    fb::awaitable<T> sync(const std::string& key, const std::function<T()>& fn)
    {
        auto subscriber = this->_pool.get();
        auto xxx = subscriber.use_count();
        rediscpp::execute(*subscriber, "subscribe", key);

        auto stream = this->_pool.get();
        auto script =
            "local success = redis.call('setnx', KEYS[1], 'lock')\n"
            "if success == 1 then\n"
            "    redis.call('expire', KEYS[1], 5)\n"
            "end\n"
            "return success";
        auto response = rediscpp::execute(*stream, "eval", script, "1", key);
        auto success = response.as<bool>();
        this->_pool.release(stream);
        if (success)
        {
            fb::logger::debug("리소스 점유에 성공");
            rediscpp::execute(*subscriber, "unsubscribe", key);
            this->_pool.release(subscriber);

            auto callback = [this, &fn, key](auto& awaitable)
            {
                auto result = fn();

                auto stream = this->_pool.get();
                rediscpp::execute(*stream, "del", key);
                rediscpp::execute(*stream, "publish", key, "ok");
                this->_pool.release(stream);

                auto thread = this->_owner.current_thread();
                if (thread != nullptr)
                {
                    thread->dispatch([&awaitable, result](uint8_t) mutable
                    {
                        awaitable.result = &result;
                        awaitable.handler.resume();
                    });
                }
                else
                {
                    awaitable.result = &result;
                    awaitable.handler.resume();
                }
            };

            return fb::awaitable<T>(callback);
        }
        else
        {
            fb::logger::debug("리소스 점유에 실패하여 대기");
            rediscpp::value value{ *subscriber };
            return std::visit(rediscpp::resp::detail::overloaded
            {
                [this, &fn, &subscriber, key](const rediscpp::resp::deserialization::array& arr)
                {
                    rediscpp::execute(*subscriber, "unsubscribe",  key);
                    this->_pool.release(subscriber);
                    return this->sync(key, fn);
                },
                [](rediscpp::resp::deserialization::error_message const& err)
                {
                    return fb::awaitable<T>([what = std::string(err.get().data())](auto& awaitable)
                    {
                        awaitable.error = std::make_unique<std::runtime_error>(what);
                        awaitable.handler.resume();
                    });
                },
                    // An unexpected response.
                    [](auto const& x)
                    {
                        return fb::awaitable<T>([](auto& awaitable)
                        {
                            awaitable.error = std::make_unique<std::runtime_error>("unhandled exception");
                            awaitable.handler.resume();
                        });
                    }
            }, value.get());
        }
    }
};

}

#endif