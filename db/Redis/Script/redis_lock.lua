local success = redis.call('setnx', @key, 'lock')
if success == 1 then
    redis.call('expire', @key, @expiry)
end
return success