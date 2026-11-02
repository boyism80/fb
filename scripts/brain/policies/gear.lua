local gear = require("brain.data.gear")

local M = {}

function M.score(query, name)
    local item = query.item(name)
    if item == nil then
        return nil
    end
    local weights = gear.weights[item.type]
    if weights == nil then
        return nil
    end

    local total = 0
    for stat, weight in pairs(weights) do
        total = total + (item[stat] or 0) * weight
    end
    return total
end

function M.worn(snap, query)
    local result = {}
    for _, slot in ipairs(snap.gear) do
        local lowest = nil
        for _, name in ipairs(slot.names) do
            local value = M.score(query, name)
            if value ~= nil and (lowest == nil or value < lowest) then
                lowest = value
            end
        end
        if result[slot.type] == nil or (lowest or -math.huge) < result[slot.type] then
            result[slot.type] = lowest or -math.huge
        end
    end
    return result
end

function M.upgrade(query, item, current)
    local value = M.score(query, item.name)
    local base  = current[item.type]
    return value ~= nil and base ~= nil and value > base, value
end

return M
