local data    = require("brain.data.combat")
local buyers  = require("brain.data.buyers")
local hunting = require("brain.data.hunting")
local items   = require("brain.data.items")
local shops   = require("brain.data.shops")
local bag     = require("brain.policies.bag")
local combat  = require("brain.policies.combat")
local danger  = require("brain.policies.danger")
local near    = require("brain.policies.near")
local party   = require("brain.policies.party")
local value   = require("brain.policies.value")

local M = {}

local capacities = nil

local function capacity(name)
    if capacities == nil then
        capacities = {}
        for _, mob in pairs(hunting.mobs) do
            for _, drop in ipairs(mob.drops or {}) do
                capacities[drop.name] = drop.capacity
            end
        end
    end
    return capacities[name] or 1
end

local function worth(snap, names)
    local keep  = bag.keep(snap)
    local found = false
    for _, name in ipairs(names) do
        if bag.room(snap, name, capacity(name)) <= 0 then
            return false
        end
        found = found or buyers[name] ~= nil or items[name] ~= nil or keep[name] ~= nil
    end
    return found
end

local function pressed(snap)
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and combat.hits(mob) and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
            return true
        end
    end
    return false
end

local function covered(snap, item)
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x == item.x and mob.y == item.y then
            return true
        end
    end
    for _, member in ipairs(party.members(snap)) do
        if member.world.map == snap.world.map and member.world.x == item.x and member.world.y == item.y then
            return true
        end
    end
    return false
end

local function priced(names)
    local total = 0
    for _, name in ipairs(names) do
        total = total + (buyers[name] ~= nil and buyers[name].price or shops[name] ~= nil and shops[name].price or 0)
    end
    return total
end

local function pile(snap, query)
    local found = nil
    local best  = nil
    local tight = pressed(snap)
    local rates = value.baseline(snap, query)
    local step  = (snap.pace.step or data.walk) / 1000
    local stuck = snap.record.memory.stuck or {}
    for _, item in ipairs(snap.world.items) do
        local cost = math.abs(item.x - snap.world.x) + math.abs(item.y - snap.world.y)
        local gain = priced(item.names) * rates.exchange - cost * step * rates.value
        if stuck[tostring(item.oid)] == nil and
            near.visible(snap, { map = snap.world.map, x = item.x, y = item.y }) and worth(snap, item.names) and
            covered(snap, item) == false and (tight == false or cost <= 1) and (best == nil or gain > best) and
            (query.warp == nil or query.warp(snap.world.map, item.x, item.y) == false) then
            found = item
            best  = gain
        end
    end
    return found
end

local function choose(snap, query)
    if snap.self.ghost then
        return nil
    end
    local item = pile(snap, query)
    if item == nil then
        return nil
    elseif item.x == snap.world.x and item.y == snap.world.y then
        local oids = {}
        for _, other in ipairs(snap.world.items) do
            if other.x == item.x and other.y == item.y then
                table.insert(oids, other.oid)
            end
        end
        return { kind = ACTION.PICK, oids = oids }
    end
    return { kind = ACTION.MOVE, dest = { map = snap.world.map, x = item.x, y = item.y, range = 0 } }
end

function M.wanted(snap, query)
    return choose(snap, query) ~= nil
end

function M.propose(snap, query)
    return choose(snap, query)
end

function M.value(snap, query)
    local item  = pile(snap, query)
    local worth = value.baseline(snap, query)
    local total = 0
    for _, name in ipairs(item.names) do
        local price = buyers[name] ~= nil and buyers[name].price or shops[name] ~= nil and shops[name].price or 0
        total = total + price
    end
    local move = { kind = ACTION.MOVE, dest = { map = snap.world.map, x = item.x, y = item.y, range = 0 } }
    local peril = value.peril(snap, query, { map = snap.world.map, x = item.x, y = item.y })
    local delta = (1 - peril) * total * worth.exchange - value.cost(snap, query, move, worth) -
        peril * danger.loss(snap, query, worth)
    if data.journal then
        log("debug",
            string.format("[brain:loot] bot=%s t=%s map=%s names=%s price=%s exchange=%s peril=%s delta=%.0f piles=%d",
                snap.self.name or "", tostring(snap.now),
                tostring(snap.world.map), table.concat(item.names, ","), tostring(total), tostring(worth.exchange),
                tostring(peril), delta, #snap.world.items))
    end
    return delta
end

return M
