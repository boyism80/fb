local gear = require("brain.policies.gear")

local M = {}

local function best(snap, query)
    local current = gear.worn(snap, query)
    local found   = nil
    for _, item in ipairs(snap.bag.items) do
        local better, value = gear.upgrade(query, item, current)
        if better and (found == nil or value > found.value) then
            found = { slot = item.slot, name = item.name, value = value }
        end
    end
    return found
end

function M.wanted(snap, query)
    return snap.self.ghost == false and snap.self.class ~= "NONE" and best(snap, query) ~= nil
end

function M.propose(snap, query)
    return { kind = ACTION.EQUIP, item = best(snap, query).name }
end

return M
