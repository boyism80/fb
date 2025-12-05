local session = redis.call('hget', @key, @name)
if session ~= false then
    redis.call('hdel', @key, @name)
    return { 1, session }
else
    return { 0 }
end

