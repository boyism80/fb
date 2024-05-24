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
    public:
    std::shared_ptr<T> get()
    {
        auto _ = std::lock_guard(_mutex);

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
    redis_stream_pool                   _pool;

public:
	redis(icontext& owner) : _owner(owner)
	{ }
	~redis() = default;

private:
    void try_lock(const std::string& key, fb::awaitable<int>& awaitable, const std::function<int()>& fn, const std::string& uuid, std::shared_ptr<cpp_redis::client> conn, std::shared_ptr<cpp_redis::subscriber> subs, fb::thread* thread);


public:
    fb::awaitable<int> sync(const std::string& key, const std::function<int()>& fn);
};

}

#endif