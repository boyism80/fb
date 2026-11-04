local M = {}

local WAYS = 64

function M.get(store, owner, base, key, compute)
    local slot = owner or ""
    local ways = store[slot]
    if ways == nil then
        ways = { count = 0, buckets = {}, older = {} }
        store[slot] = ways
    end
    local bucket = ways.buckets[base]
    if bucket == nil then
        if ways.count >= WAYS then
            ways.older   = ways.buckets
            ways.buckets = {}
            ways.count   = 0
        end
        bucket = ways.older[base] or {}
        ways.buckets[base] = bucket
        ways.count = ways.count + 1
    end
    if bucket[key] == nil then
        bucket[key] = compute()
    end
    return bucket[key]
end

return M
