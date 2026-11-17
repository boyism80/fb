local danger = require("brain.policies.danger")
local ground = require("brain.policies.ground")
local party  = require("brain.policies.party")
local score  = require("brain.policies.score")
local value  = require("brain.policies.value")
local tree   = require("brain.tree")
local board  = require("brain.tasks.board")
local group  = require("brain.tasks.group")
local learn  = require("brain.tasks.learn")
local outfit = require("brain.tasks.outfit")
local sell   = require("brain.tasks.sell")
local supply = require("brain.tasks.supply")

local choose, when, act, pick = tree.select, tree.when, tree.act, tree.pick

local function alive(snap)
    return snap.self.ghost == false
end

local function alone(snap)
    return #party.members(snap) == 0
end

local function no_group_record(snap)
    return snap.record.memory.group == nil
end

local function walk_pays(snap, facts)
    return sell.walk_worth(snap, facts) > 0
end

-- The rest of the group waits while I walk an errand, so its round trip also costs their hunting time.
-- Without this a member walked 4.5 minutes to buy gear while the group stood idle (RO+PO, 408-680s).
local function waiting(snap, facts, action)
    local members = party.members(snap)
    if action.kind ~= ACTION.MOVE or action.dest == nil or #members == 0 then
        return 0
    end
    local trip = value.trip(snap, facts, action.dest, value.baseline(snap, facts))
    if trip == nil then
        return 0
    end
    local rate = 0
    for _, member in ipairs(members) do
        if member.self.ghost == false then
            rate = rate + value.baseline(member, facts).value
        end
    end
    return 2 * trip.time * rate
end

local function errand_worth(snap, facts, found)
    local worth    = found.worth or found.task.value(snap, facts)
    local deserted = value.deserted(snap, facts, found.action)
    local leaving  = found.action.kind == ACTION.MOVE or found.action.kind == ACTION.HOLD
    local fighting = false
    for _, member in ipairs(party.members(snap)) do
        fighting = fighting or (member.task == "DEFEND" and member.self.ghost == false)
    end
    if leaving and (deserted > 0 or fighting or (snap.record.memory.servicing == nil and
            alone(snap) == false and found.name ~= "SUPPLY")) then
        return nil
    end
    -- Servicing before the ground is hunted keeps off the maps it could not cross alone: those of mobs I cannot beat
    -- (danger.hazards), and the hunting floors I would die on, hunted alone among their crowd (ground.evaluate). An errand
    -- with no way round them is left out, and one that has is walked that way; servicing begun in the hunt (leave) is not
    -- held to it (user 11-17). Told by single mobs alone, a 귀검 that beat each 흉노 crossed 흉노족2 for a gear shop, was
    -- hemmed in by thirteen and fled at 42% (online 044949, 04:51).
    local servicing = snap.record.memory.servicing
    local before    = (servicing ~= nil and servicing.leave ~= true) or
        (servicing == nil and snap.record.memory.serviced == nil)
    if before and found.action.kind == ACTION.MOVE and found.action.dest ~= nil then
        local avoided = party.perilous(snap, facts)
        for _, id in ipairs(danger.hazards(snap, facts)) do
            table.insert(avoided, id)
        end
        local dest = {}
        for key, entry in pairs(found.action.dest) do
            dest[key] = entry
        end
        local maps = dest.maps or { dest.map }
        local ends = {}
        for _, map in ipairs(maps) do
            ends[map] = true
        end
        local shunned = {}
        for _, map in ipairs(danger.shunned(snap, facts)) do
            table.insert(shunned, map)
        end
        local rested  = score.rested(snap)
        local ability = score.capability(rested, facts)
        local crossed = {}
        local route   = facts.route(maps, avoided, shunned)
        while route ~= nil do
            local deadly = nil
            for _, hop in ipairs(route.path or {}) do
                for _, map in ipairs({ hop.map, hop.dest }) do
                    local floor = map ~= snap.world.map and ends[map] == nil and crossed[map] == nil and
                        ground.floor_at(map) or nil
                    if floor ~= nil then
                        crossed[map] = true
                        local judged = ground.evaluate(rested, facts, floor, ability, nil)
                        if deadly == nil and (judged.safe == false or judged.deaths > 0) then
                            deadly = map
                        end
                    end
                end
            end
            if deadly == nil then
                break
            end
            table.insert(shunned, deadly)
            route = facts.route(maps, avoided, shunned)
        end
        if route == nil then
            return nil
        end
        dest.perilous = avoided
        dest.shunned  = shunned
        local action  = {}
        for key, entry in pairs(found.action) do
            action[key] = entry
        end
        action.dest  = dest
        found.action = action
    end
    local errand  = snap.record.memory.errand
    local started = errand ~= nil and errand.name == found.name
    local cost    = deserted + waiting(snap, facts, found.action)
    if snap.record.memory.servicing ~= nil and found.name ~= "LEARN" and found.name ~= "OUTFIT" and worth - cost <= 0 then
        return started and math.huge or 1
    end
    if worth > -math.huge and worth < math.huge then
        worth = worth - cost
        if started and worth > 0 then
            worth = math.huge
        end
    end
    return worth > 0 and worth or nil
end

local function servicing(snap)
    return snap.record.memory.servicing ~= nil
end

local function lesson_trip_ok(snap, facts)
    return learn.pays(snap, facts)
end

local function not_low_or_alone(snap, facts)
    return alone(snap) or learn.low(snap, facts) == false
end

local function free_ground(snap)
    return require("brain.policies.party").pin(snap) == nil
end

return when(alive, pick({
    when(group.offered, when(group.free_now, act("GROUP", group.propose, "invite", group.value)), "join"),
    when(board.wanted, choose({
        when(board.in_cabin, act("BOARD", board.land, "ferry_land", board.value)),
        when(board.at_sailor, act("BOARD", board.board, "ferry_board", board.value)),
        when(board.at_seller, act("BOARD", board.buy, "ferry_ticket", board.value)),
        when(board.needs_ticket, act("BOARD", board.to_seller, "to_ferry_ticket", board.value)),
        when(board.has_ticket, act("BOARD", board.to_sailor, "to_ferry_sailor", board.value)),
    }), "ferry"),
    when(free_ground, when(supply.ordered, when(supply.sold_somewhere, choose({
        when(supply.at_shop, act("SUPPLY", supply.buy, "buy_supply")),
        act("SUPPLY", supply.walk, "to_supply"),
    }))), "supply"),
    when(free_ground, when(no_group_record, when(sell.has_goods, choose({
        when(sell.at_buyer, act("SELL", sell.sell, "sell", sell.sell_worth)),
        when(alone, when(sell.has_route, when(walk_pays, act("SELL", sell.walk, "to_buyer", sell.walk_worth)))),
    }))), "sell"),
    when(free_ground, when(servicing, when(sell.has_goods, choose({
        when(sell.at_buyer, act("SELL", sell.sell, "sell_servicing", sell.sell_worth)),
        when(sell.has_route, act("SELL", sell.walk, "to_buyer_servicing", sell.walk_worth)),
    }))), "sell_servicing"),
    when(free_ground, when(outfit.has_plan, when(outfit.pays, act("OUTFIT", outfit.next_step, "buy_gear"))), "outfit"),
    when(free_ground, when(learn.has_lesson,
        when(not_low_or_alone, when(lesson_trip_ok, act("LEARN", learn.next_step, "study")))), "learn"),
}, errand_worth), "errand")
