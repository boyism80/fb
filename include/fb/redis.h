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

/**
 * @brief      Connection pool for Redis database connections.
 *
 *             Manages a pool of Redis connections to improve performance by reusing
 *             connections instead of creating new ones for each operation. The pool
 *             maintains a fixed number of connections and provides thread-safe
 *             access to available connections.
 */
class redis_connection_pool
{
    const char*                _host; ///< Redis server hostname or IP address
    int                        _port; ///< Redis server port number
    int                        _size; ///< Maximum number of connections in the pool
    std::vector<redisContext*> _pool; ///< Pool of available Redis connections

public:
    /**
     * @brief      Constructs a Redis connection pool.
     *
     * @param[in]  host  The Redis server hostname or IP address.
     * @param[in]  port  The Redis server port number.
     * @param[in]  size  The number of connections to maintain in the pool.
     */
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

    /**
     * @brief      Destructor that cleans up all Redis connections.
     */
    ~redis_connection_pool()
    {
        for (auto context : this->_pool)
        {
            redisFree(context);
        }
    }

    /**
     * @brief      Gets an available Redis connection from the pool.
     *
     * @return     A pointer to a Redis context, or nullptr if no connections are available.
     */
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

    /**
     * @brief      Returns a Redis connection to the pool for reuse.
     *
     * @param      context  The Redis context to return to the pool.
     */
    void put(redisContext* context)
    {
        this->_pool.push_back(context);
    }
};

/**
 * @brief      High-level Redis client with connection pooling and type-safe operations.
 *
 *             This class provides a convenient interface for Redis operations with
 *             automatic connection management, type conversion, and error handling.
 *             It supports various data types including strings, integers, doubles,
 *             booleans, and vectors through template-based command execution.
 */
class redis
{
public:
    redis_connection_pool _pool; ///< Connection pool for managing Redis connections

public:
    /**
     * @brief      Constructs a Redis client with connection pooling.
     *
     * @param[in]  host  The Redis server hostname or IP address.
     * @param[in]  port  The Redis server port number.
     * @param[in]  size  The number of connections to maintain in the pool.
     */
    redis(const char* host, int port, int size) :
        _pool(host, port, size)
    { }

    /**
     * @brief      Copy constructor is deleted to prevent copying.
     */
    redis(const redis&) = delete;

    /**
     * @brief      Move constructor is deleted to prevent moving.
     */
    redis(redis&&) = delete;

    /**
     * @brief      Destructor is defaulted.
     */
    ~redis() = default;

private:
    /**
     * @brief      Converts a Redis reply to the specified C++ type.
     *
     *             Provides type-safe conversion from Redis reply objects to C++ types.
     *             Supports strings, integers, doubles, booleans, and vectors of these types.
     *
     * @param      reply  The Redis reply to convert.
     *
     * @tparam     T      The target C++ type for conversion.
     *
     * @return     The converted value of type T.
     *
     * @throws     std::runtime_error if the reply type doesn't match the expected type.
     */
    template <typename T>
    static T as(redisReply* reply)
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
    /**
     * @brief      Executes a Redis command and returns the result as the specified type.
     *
     *             Sends a command to Redis using an available connection from the pool,
     *             converts the reply to the specified C++ type, and returns the connection
     *             to the pool. Supports void return type for commands that don't return values.
     *
     * @param[in]  cmd   The Redis command string to execute.
     *
     * @tparam     T     The expected return type (void for commands without return values).
     *
     * @return     The command result converted to type T.
     *
     * @throws     std::runtime_error if connection fails or command execution fails.
     */
    template <typename T>
    T command(const std::string& cmd)
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