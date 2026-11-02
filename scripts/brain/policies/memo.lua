local M = {}

function M.get(store, owner, base, key, compute)
    local slot   = owner or ""
    local bucket = store[slot]
    if bucket == nil or bucket.base ~= base then
        bucket      = { base = base, entries = {} }
        store[slot] = bucket
    end
    if bucket.entries[key] == nil then
        bucket.entries[key] = compute()
    end
    return bucket.entries[key]
end

return M
