local session = redis.call('hget', @key, @name)
if session ~= false then
	redis.call('hdel', @key, @name)
	return { 0, session }
else
	redis.call('hset', @key, @name, @session)
	return { 1 }
end