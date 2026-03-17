local COUNT_REFS = KEYS[1]
local EXPIRY = tonumber(ARGV[1])
local LENGTH = tonumber(ARGV[2])

for i = 1, LENGTH do
    local field = ARGV[2 * i + 1]
    local count = tonumber(ARGV[2 * i + 2])
    if count == nil then
        count = 0
    end
    local ref = redis.call('hincrby', COUNT_REFS, field, -count)
    if tonumber(ref) <= 0 then
        redis.call('hdel', COUNT_REFS, field)
        redis.call('expire', field, EXPIRY)
    end
end