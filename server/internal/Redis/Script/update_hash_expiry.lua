local CACHE_KEY = KEYS[1]
local COUNT_REFS = KEYS[2]
local EXPIRY = tonumber(ARGV[1])
local LENGTH = tonumber(ARGV[2])

local offset = 2
for i = 1, LENGTH do
	local field = ARGV[offset + i]
	local value = ARGV[offset + i + 1]
	redis.call('hset', CACHE_KEY, field, value)
	offset = offset + 1
end

local contains_refs = redis.call('hexists', COUNT_REFS, CACHE_KEY)
if contains_refs == 0 then
	redis.call('expire', CACHE_KEY, EXPIRY)
end

return {contains_refs}