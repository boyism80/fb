local combat = require("brain.policies.combat")
local danger = require("brain.policies.danger")
local value  = require("brain.policies.value")

local M = {}

local RECENT = 2

local function attacker(snap, query)
    local found = nil
    for _, mob in ipairs(snap.world.mobs) do
        local near = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) <= 1
        if near and combat.hits(mob) and snap.self.struck ~= nil and snap.self.struck <= (mob.speed or 0) * RECENT and
            danger.winnable(snap, query, mob) then
            found = mob
        end
    end
    return found
end

function M.wanted(snap, query)
    return snap.self.ghost == false and attacker(snap, query) ~= nil
end

function M.propose(snap, query)
    local mob = attacker(snap, query)
    return { kind = ACTION.ATTACK, oid = mob.oid, tactic = combat.plan(snap, query, mob, danger.guard(mob)) }
end

function M.value(snap, query)
    local mob   = attacker(snap, query)
    local worth = value.baseline(snap, query)
    local fight = combat.fight(snap, query, mob)
    return math.max(0, (mob.exp or 0) + danger.loss(snap, query, worth) -
        value.cost(snap, query, M.propose(snap, query), worth, fight.time))
end

return M
