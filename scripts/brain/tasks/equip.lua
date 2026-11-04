local combat = require("brain.policies.combat")
local gear   = require("brain.policies.gear")
local value  = require("brain.policies.value")

local M = {}

local function best(snap, query)
    local current = gear.worn(snap, query)
    for _, slot in ipairs(snap.gear or {}) do
        if #slot.names > 0 and current[slot.type] ~= -math.huge then
            current[slot.type] = math.max(current[slot.type] or -math.huge,
                gear.score(query, slot.names[1]) or -math.huge)
        end
    end
    local found   = nil
    local refused = snap.record.memory.refused or {}
    for _, item in ipairs(snap.bag.items) do
        local better, value = gear.upgrade(query, item, current)
        if better and (refused[item.name] or 0) < snap.self.level and (found == nil or value > found.value) then
            found = { slot = item.slot, name = item.name, type = item.type, value = value }
        end
    end
    return found
end

function M.wanted(snap, query)
    return snap.self.ghost == false and snap.self.class ~= "NONE" and best(snap, query) ~= nil and
        M.value(snap, query) >= 0
end

function M.value(snap, query)
    local found = best(snap, query)
    local slots = {}
    for _, slot in ipairs(snap.gear or {}) do
        if slot.type ~= found.type then
            table.insert(slots, slot)
        end
    end
    table.insert(slots, { type = found.type, names = { found.name } })
    local after = setmetatable({ gear = slots }, { __index = snap })
    local power = value.power(snap, query)
    local ratio = power > 0 and value.power(after, query) / power - 1 or 0
    local worth = value.baseline(snap, query)
    if ratio < 0 then
        return ratio
    elseif ratio == 0 or worth.value <= 0 then
        return 0
    end
    return ratio * worth.value * combat.cast(snap) / 1000
end

function M.propose(snap, query)
    return { kind = ACTION.EQUIP, item = best(snap, query).name }
end

return M
