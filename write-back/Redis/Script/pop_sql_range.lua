if redis.call('exists', @key) == false then
	return {}
end
local values = redis.call('lrange', @key, 0, @count - 1)
redis.call('ltrim', @key, @count, -1)
return values