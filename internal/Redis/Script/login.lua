if redis.call('hexists', @key, @uid) == 1 then
	return { 0 }
else
	redis.call('hset', @key, @uid, 1)
	return { 1 }
end