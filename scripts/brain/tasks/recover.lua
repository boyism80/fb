local danger = require("brain.policies.danger")
local value  = require("brain.policies.value")

local M = {}

local WEARABLE = { WEAPON = true, ARMOR = true, HELMET = true, RING = true, SHIELD = true, AUXILIARY = true }
local BARRED   = { BLOCKED = true, OCCUPIED = true, UNSAFE = true }

local function holding(snap)
    local counts = {}
    for _, item in ipairs(snap.bag.items) do
        counts[item.name] = (counts[item.name] or 0) + item.count
    end
    return counts
end

function M.piles(snap)
    local result = {}
    for _, item in ipairs(snap.world.items) do
        if item.x == snap.world.x and item.y == snap.world.y then
            table.insert(result, item.oid)
        end
    end
    return result
end

function M.spare(snap)
    local death  = snap.record.death
    local counts = holding(snap)
    for _, item in ipairs(snap.bag.items) do
        if WEARABLE[item.type] and counts[item.name] > (death.items[item.name] or 0) then
            return item.name
        end
    end
    return nil
end

local function missing(snap, death)
    local counts = holding(snap)
    if snap.self.money < death.money then
        return true
    end
    for name, count in pairs(death.items) do
        if (counts[name] or 0) < count then
            return true
        end
    end
    return false
end

function M.exhausted(snap)
    local death  = snap.record.death
    local failed = snap.record.failures.RECOVER
    return death.left <= 0 or missing(snap, death) == false or (failed ~= nil and BARRED[failed.reason] == true)
end

function M.profit(snap, query, action)
    local death  = snap.record.death
    local worth  = value.baseline(snap, query)
    local counts = holding(snap)
    local lost   = math.max(0, death.money - snap.self.money)
    for name, count in pairs(death.items) do
        local item = query.item(name)
        lost = lost + math.max(0, count - (counts[name] or 0)) * (item ~= nil and item.price or 0)
    end
    local peril = value.peril(snap, query, { map = death.map, x = death.x, y = death.y })
    return (1 - peril) * lost * worth.exchange - value.cost(snap, query, action, worth) -
        peril * danger.loss(snap, query, worth)
end

return M
