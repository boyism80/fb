local COUNT_REFS = KEYS[1]
local EXPIRY = tonumber(ARGV[1])
local LENGTH = tonumber(ARGV[2])

local offset = 2
for i = 1, LENGTH do
    local field = ARGV[offset + i]
    local count = tonumber(ARGV[offset + i + 1])
    local ref = redis.call('hincrby', COUNT_REFS, field, count * -count)
    if tonumber(ref) <= 0 then
        redis.call('hdel', COUNT_REFS, field)
        redis.call('expire', field, EXPIRY)
    end
    offset = offset + 1
end