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

local spotted = {}

local function yielding(snap, item)
    local mine = snap.record.memory.looted or 0
    local step = snap.pace.step or data.walk
    local key  = table.concat({ tostring(snap.record.memory.lane), snap.self.name or "", item.oid }, "|")
    for _, member in ipairs(party.members(snap)) do
        local world = member.world
        if member.self.ghost == false and world.map == snap.world.map and world.x ~= nil and
            (member.looted or 0) < mine and (member.bag.capacity or 0) > (member.bag.used or 0) then
            spotted[key] = spotted[key] or snap.now
            local patience = ((math.abs(world.x - item.x) + math.abs(world.y - item.y)) * step + data.swing) *
                (1 + mine - (member.looted or 0))
            if snap.now - spotted[key] < patience then
                return true
            end
        end
    end
    return false
end

function M.pile(snap, query)
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
            yielding(snap, item) == false and
            (query.warp == nil or query.warp(snap.world.map, item.x, item.y) == false) then
            found = item
            best  = gain
        end
    end
    return found
end

function M.on_pile(snap, query)
    local item = M.pile(snap, query)
    return item.x == snap.world.x and item.y == snap.world.y
end

function M.pick(snap, query)
    local item = M.pile(snap, query)
    local oids = {}
    for _, other in ipairs(snap.world.items) do
        if other.x == item.x and other.y == item.y then
            table.insert(oids, other.oid)
        end
    end
    return { kind = ACTION.PICK, oids = oids, looted = (snap.record.memory.looted or 0) + #oids }
end

function M.walk(snap, query)
    local item = M.pile(snap, query)
    return { kind = ACTION.MOVE, dest = { map = snap.world.map, x = item.x, y = item.y, range = 0 } }
end

function M.value(snap, query)
    local item  = M.pile(snap, query)
    local worth = value.baseline(snap, query)
    local total = 0
    for _, name in ipairs(item.names) do
        local price = buyers[name] ~= nil and buyers[name].price or shops[name] ~= nil and shops[name].price or 0
        total = total + price
    end
    local move  = { kind = ACTION.MOVE, dest = { map = snap.world.map, x = item.x, y = item.y, range = 0 } }
    local peril = value.peril(snap, query, { map = snap.world.map, x = item.x, y = item.y })
    local lead  = 0
    for _, member in ipairs(party.members(snap)) do
        lead = math.max(lead, (member.looted or 0) - (snap.record.memory.looted or 0))
    end
    local delta = (1 + lead) * (1 - peril) * total * worth.exchange - value.cost(snap, query, move, worth) / (1 + lead) -
        peril * danger.loss(snap, query, worth) / (1 + lead)
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
