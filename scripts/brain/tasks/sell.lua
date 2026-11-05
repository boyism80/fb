local buyers   = require("brain.data.buyers")
local items    = require("brain.data.items")
local teleport = require("brain.data.teleport")
local bag      = require("brain.policies.bag")
local gear     = require("brain.policies.gear")
local near     = require("brain.policies.near")
local party    = require("brain.policies.party")
local value    = require("brain.policies.value")

local M = {}

local function movers()
    local found = {}
    for _, item in ipairs(teleport.items) do
        found[item.name] = true
    end
    return found
end

local function goods(snap, query)
    local usable  = bag.usable(snap)
    local skip    = movers()
    local current = gear.worn(snap, query)
    local found   = {}
    for _, item in ipairs(snap.bag.items) do
        local name  = bag.unpack(item)
        local buyer = buyers[name]
        if buyer ~= nil and items[name] == nil and skip[name] == nil and gear.upgrade(query, item, current) == false and
            (usable[name] or 0) > 0 then
            table.insert(found, {
                name = name,
                value = buyer.price * usable[name],
                npcs = buyer.npcs,
                whole = usable[name] >= item.count
            })
        end
    end
    table.sort(found, function(left, right)
        return left.value > right.value or (left.value == right.value and left.name < right.name)
    end)
    return found
end

local function trip(snap, query, good, worth)
    local maps = {}
    for _, spot in ipairs(query.npcs(good.npcs)) do
        table.insert(maps, spot.map)
    end
    if #maps == 0 then
        return nil
    end
    return value.detour(snap, query, { maps = maps }, worth)
end

local function lost(snap, yields, horizon, freed, sold)
    local loss = 0
    for name, entry in pairs(yields) do
        local buyer = buyers[name]
        if buyer ~= nil and entry.rate > 0 then
            local room = sold[name] and (math.max(0, snap.bag.capacity - snap.bag.used) + freed) * entry.capacity or
                bag.room(snap, name, entry.capacity, freed)
            loss = loss + buyer.price * math.max(0, entry.rate * horizon - room)
        end
    end
    return loss
end

local function gain(snap, query, found, route, worth)
    local length = route ~= nil and route.time or 0
    local risk   = route ~= nil and route.risk or 0
    local hunt   = worth.hunt
    if hunt == nil or hunt.yields == nil or (hunt.loot or 0) <= 0 then
        return -length * worth.value - risk
    end
    local freed = 0
    local sold  = {}
    for _, good in ipairs(found) do
        sold[good.name] = true
        freed = freed + (good.whole and 1 or 0)
    end
    local window = (math.max(0, snap.bag.capacity - snap.bag.used) + freed) / hunt.loot
    local stay   = lost(snap, hunt.yields, window, 0, {})
    local after  = lost(snap, hunt.yields, math.max(0, window - length), freed, sold)
    return (stay - after) * worth.exchange - length * worth.value - risk
end

local function choose(snap, query)
    if snap.self.ghost or snap.record.memory.group ~= nil then
        return nil
    end
    local found = goods(snap, query)
    if #found == 0 then
        return nil
    end
    local worth = value.baseline(snap, query)
    for _, good in ipairs(found) do
        local spot = near.approach(snap, query, good.npcs)
        if spot ~= nil then
            return { kind = ACTION.SAY, text = string.format("%s 다 판다", good.name), lose = good.name },
                math.max(0, gain(snap, query, found, nil, worth))
        end
    end
    local good  = found[1]
    local route = trip(snap, query, good, worth)
    if route == nil then
        return nil
    end
    local delta = gain(snap, query, found, route, worth)
    if delta <= 0 then
        return nil
    end
    if #party.members(snap) > 0 then
        return nil
    end
    local _, move = near.approach(snap, query, good.npcs)
    return move, delta
end

function M.wanted(snap, query)
    return choose(snap, query) ~= nil
end

function M.propose(snap, query)
    return (choose(snap, query))
end

function M.value(snap, query)
    local _, delta = choose(snap, query)
    return delta or 0
end

return M
