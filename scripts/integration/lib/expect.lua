local M = {}

local INTERVAL   = 100
local FAIL_LIMIT = 8

function M.within(ctx, timeout, predicate)
    local waited = 0
    while predicate() == false do
        if waited >= timeout then
            return false
        end
        ctx:sleep(INTERVAL)
        waited = waited + INTERVAL
    end
    return true
end

function M.arrive(ctx, index, name, timeout)
    local arrived = M.within(ctx, timeout, function()
        local model = id2map(ctx:bot(index):map())
        return model ~= nil and model:name() == name
    end)
    return arrived, ctx:bot(index)
end

function M.settle(ctx, index, mind, timeout, predicate, watch)
    local stuck = false
    local done  = M.within(ctx, timeout, function()
        local current = ctx:bot(index)
        if watch ~= nil then
            watch(current)
        end
        local state = mind:state(current)
        stuck       = state ~= nil and (state.failures >= FAIL_LIMIT or mind:observe(current).online == false)
        return stuck or (state ~= nil and predicate(current))
    end)
    return done and stuck == false
end

function M.release(ctx, index, mind, timeout)
    local held = M.within(ctx, timeout, function()
        return mind:state(ctx:bot(index)) ~= nil
    end)
    ctx:bot(index):brain(nil)
    return held
end

return M
