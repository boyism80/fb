local data    = require("brain.data.combat")
local battle  = require("brain.policies.battle")
local combat  = require("brain.policies.combat")
local danger  = require("brain.policies.danger")
local failure = require("brain.policies.failure")
local party   = require("brain.policies.party")
local aid     = require("brain.tasks.aid")
local field   = require("brain.tasks.field")
local value   = require("brain.policies.value")

local M = {}

local RECENT = 2

local function attacker(snap, query)
    return battle.target(snap, query, function(mob)
        local near = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) <= 1
        return near and combat.hits(mob) and snap.self.struck ~= nil and snap.self.struck <= (mob.speed or 0) * RECENT
    end)
end

function M.wanted(snap, query)
    return snap.self.ghost == false and attacker(snap, query) ~= nil
end

function M.propose(snap, query)
    return battle.propose(attacker(snap, query))
end

local function barring(snap, query)
    return battle.target(snap, query, function(mob)
        return math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) <= 1
    end)
end

function M.walled_in(snap)
    local blocked = snap.record.memory.blocked
    return snap.self.ghost == false and type(blocked) == "table" and blocked.map == snap.world.map and
        snap.now - blocked.at < (snap.pace.swing or data.swing)
end

function M.barred(snap, query)
    return barring(snap, query) ~= nil
end

function M.clear_path(snap, query)
    return battle.propose(barring(snap, query))
end

local function passing(snap, query, mob, fight)
    local plan = party.plan(snap, query)
    if plan == nil or failure.remaining(snap, "FIELD") ~= 0 or field.wanted(snap, query) == false then
        return false
    end
    local blocked = snap.record.memory.blocked
    if type(blocked) == "table" and snap.now - blocked.at < fight.time then
        return false
    end
    if plan.gate == snap.world.map then
        return false
    end
    for _, floor in ipairs(plan.floors) do
        if floor.map == snap.world.map then
            return false
        end
    end
    return fight.time > (mob.speed or 0) and combat.gain(mob, snap) / math.max(fight.time, 1) * 1000 < plan.rate
end

local function marginal(found, snap)
    local mob = found.mob
    return (found.choice.damage or 0) / math.max(mob.hp or 1, 1) * combat.gain(mob, snap)
end

function M.value(snap, query)
    local found = attacker(snap, query)
    local worth = value.baseline(snap, query)
    local fight = combat.outcome(snap, query, found.mob)
    if passing(snap, query, found.mob, fight) then
        return -1
    end
    local aiding  = aid.wanted(snap, query)
    local ignored = combat.lethal(snap, found.mob, aiding and combat.cast(snap) or fight.time, snap.self.hp)
    local gain    = found.choice.value
    if gain > 0 and aiding and aid.value(snap, query) > marginal(found, snap) then
        gain = math.min(gain, aid.value(snap, query) - 1)
    end
    return math.max(0, gain + ignored * danger.loss(snap, query, worth))
end

function M.attacked(snap, query)
    return attacker(snap, query) ~= nil
end

function M.holds_ground(snap, query)
    local found = attacker(snap, query)
    return passing(snap, query, found.mob, combat.outcome(snap, query, found.mob)) == false
end

function M.strike_back(snap, query)
    return battle.propose(attacker(snap, query))
end

return M
