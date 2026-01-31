if redis.call('exists', @lock_key) == 0 then
    redis.call('set', @lock_key, '1')
    redis.call('expire', @lock_key, @ttl_seconds)
    return 1
else
    return 0
end
