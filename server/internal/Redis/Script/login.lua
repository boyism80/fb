local session = redis.call('hget', @key, @name)
if session ~= false then
    redis.call('hdel', @key, @name)
    return { 0, session }
else
    redis.call('hset', @key, @name, @session)
    redis.call('expire', @key, @ttl)
    return { 1 }
end