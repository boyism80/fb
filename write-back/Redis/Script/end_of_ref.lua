local ref = redis.call('hincrby', @key, @field, @count * -1)
if tonumber(ref) == 0 then
    redis.call('hdel', @key, @field)
    redis.call('expire', @field, @expiry)
end

return { ref }