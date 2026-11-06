local M = {}

local WAYS = 64

function M.peek(store, owner, base, key)
    local ways = store[owner or ""]
    if ways == nil then
        return nil
    end
    local bucket = ways.buckets[base] or ways.older[base]
    return bucket ~= nil and bucket[key] or nil
end

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
