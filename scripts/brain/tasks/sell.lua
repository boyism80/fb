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

function M.has_goods(snap, query)
    return #goods(snap, query) > 0
end

local function buyer_near(snap, query)
    for _, good in ipairs(goods(snap, query)) do
        if near.approach(snap, query, good.npcs) ~= nil then
            return good
        end
    end
    return nil
end

function M.at_buyer(snap, query)
    return buyer_near(snap, query) ~= nil
end

function M.sell(snap, query)
    local good = buyer_near(snap, query)
    return { kind = ACTION.SAY, text = string.format("%s 다 판다", good.name), lose = good.name }
end

function M.sell_worth(snap, query)
    return math.max(0, gain(snap, query, goods(snap, query), nil, value.baseline(snap, query)))
end

local function route_to_buyer(snap, query)
    return trip(snap, query, goods(snap, query)[1], value.baseline(snap, query))
end

function M.has_route(snap, query)
    return route_to_buyer(snap, query) ~= nil
end

function M.walk_worth(snap, query)
    return gain(snap, query, goods(snap, query), route_to_buyer(snap, query), value.baseline(snap, query))
end

function M.walk(snap, query)
    local _, move = near.approach(snap, query, goods(snap, query)[1].npcs)
    return move
end

return M
