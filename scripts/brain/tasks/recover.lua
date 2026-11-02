local value = require("brain.policies.value")

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

local function piles(snap)
    local result = {}
    for _, item in ipairs(snap.world.items) do
        if item.x == snap.world.x and item.y == snap.world.y then
            table.insert(result, item.oid)
        end
    end
    return result
end

local function spare(snap, death)
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

function M.wanted(snap)
    return snap.self.ghost == false and snap.record.death ~= nil
end

local function plan(snap, query)
    local death  = snap.record.death
    local worn   = spare(snap, death)
    local found  = piles(snap)
    local failed = snap.record.failures.RECOVER
    if worn ~= nil then
        return { kind = ACTION.EQUIP, item = worn }
    elseif death.left <= 0 or missing(snap, death) == false or (failed ~= nil and BARRED[failed.reason]) then
        return { kind = ACTION.FORGET }
    elseif snap.world.map ~= death.map or snap.world.x ~= death.x or snap.world.y ~= death.y then
        return { kind = ACTION.MOVE, dest = { map = death.map, x = death.x, y = death.y, range = 0 } }
    elseif #found > 0 then
        return { kind = ACTION.PICK, oids = found }
    else
        return { kind = ACTION.FORGET }
    end
end

local function profit(snap, query, action)
    local death  = snap.record.death
    local worth  = value.baseline(snap, query)
    local counts = holding(snap)
    local lost   = math.max(0, death.money - snap.self.money)
    for name, count in pairs(death.items) do
        local item = query.item(name)
        lost = lost + math.max(0, count - (counts[name] or 0)) * (item ~= nil and item.price or 0)
    end
    return lost * worth.exchange - value.cost(snap, query, action, worth) -
        value.spot(snap, query, { map = death.map, x = death.x, y = death.y }, worth)
end

function M.propose(snap, query)
    local action = plan(snap, query)
    if action.kind ~= ACTION.FORGET and profit(snap, query, action) <= 0 then
        return { kind = ACTION.FORGET }
    end
    return action
end

function M.value(snap, query)
    local action = plan(snap, query)
    if action.kind == ACTION.FORGET then
        return 0
    end
    return math.max(0, profit(snap, query, action))
end

return M
