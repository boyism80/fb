local ferry = require("brain.policies.ferry")
local near  = require("brain.policies.near")
local party = require("brain.policies.party")

local M = {}

local function held(snap, name)
    for _, item in ipairs(snap.bag.items or {}) do
        if item.name == name and (item.count or 0) > 0 then
            return true
        end
    end
    return false
end

local function route_of(snap, query)
    local plan = party.plan(snap, query)
    if plan == nil or plan.floors == nil or plan.floors[1] == nil then
        return nil
    end
    return query.path({ maps = { plan.floors[1].map } }, {}, {}, {
        map = snap.world.map,
        x = snap.world.x or 0,
        y = snap.world.y or 0,
        level = snap.self.level,
    })
end

local function active(snap, query)
    local path = route_of(snap, query)
    return ferry.needed(path ~= nil and path.path or nil)
end

function M.wanted(snap, query)
    return active(snap, query) ~= nil
end

function M.needs_ticket(snap, query)
    local route = active(snap, query)
    return route ~= nil and held(snap, route.ticket) == false
end

function M.has_ticket(snap, query)
    local route = active(snap, query)
    return route ~= nil and held(snap, route.ticket) and snap.world.map ~= route.cabin.map
end

function M.at_seller(snap, query)
    local route = active(snap, query)
    if route == nil or held(snap, route.ticket) then
        return false
    end
    return near.approach(snap, query, { route.seller }) ~= nil
end

function M.at_sailor(snap, query)
    local route = active(snap, query)
    if route == nil or held(snap, route.ticket) == false then
        return false
    end
    if snap.world.map == route.cabin.map then
        return false
    end
    return near.approach(snap, query, { route.sailor }) ~= nil
end

function M.in_cabin(snap, query)
    local route = active(snap, query)
    return route ~= nil and snap.world.map == route.cabin.map
end

function M.buy(snap, query)
    local route = active(snap, query)
    return {
        kind = ACTION.SAY,
        text = string.format("%s 1개 줘", route.ticket),
        gain = route.ticket,
    }
end

function M.to_seller(snap, query)
    local route = active(snap, query)
    local _, move = near.approach(snap, query, { route.seller })
    return move or { kind = ACTION.HOLD }
end

function M.to_sailor(snap, query)
    local route = active(snap, query)
    local _, move = near.approach(snap, query, { route.sailor })
    return move or { kind = ACTION.HOLD }
end

function M.board(snap, query)
    local route = active(snap, query)
    return {
        kind = ACTION.TALK,
        npc = route.sailor,
        answers = route.board_menu,
        expect = { map = route.cabin.map },
    }
end

function M.land(snap, query)
    local route = active(snap, query)
    return {
        kind = ACTION.TALK,
        npc = route.boatman,
        answers = {},
        expect = { map = route.land.map },
    }
end

function M.value(snap, query)
    return active(snap, query) ~= nil and math.huge or 0
end

return M
