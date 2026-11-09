local party  = require("brain.policies.party")
local value  = require("brain.policies.value")
local tree   = require("brain.tree")
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

return when(alive, pick({
    when(group.offered, when(group.free_now, act("GROUP", group.propose, "invite", group.value)), "join"),
    when(supply.ordered, when(supply.sold_somewhere, choose({
        when(supply.at_shop, act("SUPPLY", supply.buy, "buy_supply")),
        act("SUPPLY", supply.walk, "to_supply"),
    })), "supply"),
    when(no_group_record, when(sell.has_goods, choose({
        when(sell.at_buyer, act("SELL", sell.sell, "sell", sell.sell_worth)),
        when(alone, when(sell.has_route, when(walk_pays, act("SELL", sell.walk, "to_buyer", sell.walk_worth)))),
    })), "sell"),
    when(servicing, when(sell.has_goods, choose({
        when(sell.at_buyer, act("SELL", sell.sell, "sell_servicing", sell.sell_worth)),
        when(sell.has_route, act("SELL", sell.walk, "to_buyer_servicing", sell.walk_worth)),
    })), "sell_servicing"),
    when(outfit.has_plan, when(outfit.pays, act("OUTFIT", outfit.next_step, "buy_gear")), "outfit"),
    when(learn.has_lesson,
        when(not_low_or_alone, when(lesson_trip_ok, act("LEARN", learn.next_step, "study"))), "learn"),
}, errand_worth), "errand")
