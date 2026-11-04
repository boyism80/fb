local buyers  = require("brain.data.buyers")
local hunting = require("brain.data.hunting")
local items   = require("brain.data.items")
local shops   = require("brain.data.shops")
local bag     = require("brain.policies.bag")
local danger  = require("brain.policies.danger")
local near    = require("brain.policies.near")
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

local function pile(snap)
    local found = nil
    local best  = nil
    for _, item in ipairs(snap.world.items) do
        local cost = math.abs(item.x - snap.world.x) + math.abs(item.y - snap.world.y)
        if near.visible(snap, { map = snap.world.map, x = item.x, y = item.y }) and worth(snap, item.names) and
            (best == nil or cost < best) then
            found = item
            best  = cost
        end
    end
    return found
end

local function choose(snap)
    if snap.self.ghost then
        return nil
    end
    local item = pile(snap)
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

function M.wanted(snap)
    return choose(snap) ~= nil
end

function M.propose(snap)
    return choose(snap)
end

function M.value(snap, query)
    local item  = pile(snap)
    local worth = value.baseline(snap, query)
    local total = 0
    for _, name in ipairs(item.names) do
        local price = buyers[name] ~= nil and buyers[name].price or shops[name] ~= nil and shops[name].price or 0
        total = total + price
    end
    local move = { kind = ACTION.MOVE, dest = { map = snap.world.map, x = item.x, y = item.y, range = 0 } }
    local peril = value.peril(snap, query, { map = snap.world.map, x = item.x, y = item.y })
    return (1 - peril) * total * worth.exchange - value.cost(snap, query, move, worth) -
        peril * danger.loss(snap, query, worth)
end

return M
