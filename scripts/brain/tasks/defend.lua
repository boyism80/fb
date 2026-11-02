local battle = require("brain.policies.battle")
local combat = require("brain.policies.combat")
local danger = require("brain.policies.danger")
local value  = require("brain.policies.value")

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

function M.value(snap, query)
    local found   = attacker(snap, query)
    local worth   = value.baseline(snap, query)
    local fight   = combat.outcome(snap, query, found.mob)
    local ignored = combat.lethal(snap, found.mob, fight.time, snap.self.hp)
    return math.max(0, found.choice.value + ignored * danger.loss(snap, query, worth))
end

return M
