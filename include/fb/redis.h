#ifndef __REDIS_H__
#define __REDIS_H__

#include <hiredis/hiredis.h>
#include <vector>
#include <fb/logger.h>

namespace fb {

namespace detail {
template <typename> constexpr bool   is_vector_impl                 = false;
template <typename T> constexpr bool is_vector_impl<std::vector<T>> = true;
template <typename T> constexpr bool is_vector                      = is_vector_impl<std::decay_t<T>>;
} // namespace detail

class redis_connection_pool
{
    const char*                _host;
    int                        _port;
    int                        _size;
    std::vector<redisContext*> _pool;

public:
    redis_connection_pool(const char* host, int port, int size) :
        _host(host),
        _port(port),
        _size(size)
    {
        for (int i = 0; i < this->_size; ++i)
        {
            auto context = redisConnect(host, port);
            if (context == nullptr || context->err)
            {
                fb::logger::fatal("Redis connection failed");
                return;
            }
            this->_pool.push_back(context);
        }
    }

    ~redis_connection_pool()
    {
        for (auto context : this->_pool)
        {
            redisFree(context);
        }
    }

    redisContext* get()
    {
        if (this->_pool.empty())
        {
            return nullptr;
        }
        auto context = this->_pool.back();
        this->_pool.pop_back();
        return context;
    }

    void put(redisContext* context)
    {
        this->_pool.push_back(context);
    }
};

class redis
{
public:
    redis_connection_pool _pool;

public:
    redis(const char* host, int port, int size) :
        _pool(host, port, size)
    { }

    redis(const redis&) = delete;
    redis(redis&&)      = delete;
    ~redis()            = default;

private:
    template <typename T> static T as(redisReply* reply)
    {
        if constexpr (detail::is_vector<T>)
        {
            if (reply->type != REDIS_REPLY_ARRAY)
                throw std::runtime_error("redis reply type mismatch");

            auto values = std::vector<typename T::value_type>{};
            for (size_t i = 0; i < reply->elements; ++i)
            {
                auto element = reply->element[i];
                values.push_back(as<typename T::value_type>(element));
            }
            return values;
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            if (reply->type != REDIS_REPLY_STRING)
                throw std::runtime_error("redis reply type mismatch");
            return std::string(reply->str, reply->len);
        }
        else if constexpr (std::is_same_v<T, int>)
        {
            if (reply->type != REDIS_REPLY_INTEGER)
                throw std::runtime_error("redis reply type mismatch");
            return reply->integer;
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            if (reply->type != REDIS_REPLY_DOUBLE)
                throw std::runtime_error("redis reply type mismatch");
            return reply->dval;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            if (reply->type != REDIS_REPLY_INTEGER)
                throw std::runtime_error("redis reply type mismatch");
            return reply->integer != 0;
        }
        else
        {
            throw std::runtime_error("unsupported type");
        }
    }

public:
    template <typename T> T command(const std::string& cmd)
    {
        auto context = this->_pool.get();
        if (context == nullptr)
            throw std::runtime_error("redis connection failed");

        try
        {
            auto reply = (redisReply*)redisCommand(context, cmd.c_str());
            if (reply == nullptr)
                throw std::runtime_error("redis command failed");

            if constexpr (std::is_void_v<T>)
            {
                freeReplyObject(reply);
                this->_pool.put(context);
                return;
            }
            else
            {
                auto&& result = redis::as<T>(reply);
                this->_pool.put(context);
                return result;
            }
        }
        catch (std::exception& e)
        {
            this->_pool.put(context);
            throw e;
        }
    }
};

} // namespace fb

#endif