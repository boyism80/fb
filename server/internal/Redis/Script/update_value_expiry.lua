redis.call('set', @key, @value)

local contains_refs = redis.call('hexists', @cref, @key)
if contains_refs == 0 then
	redis.call('expire', @key, @expiry)
end

return contains_refs