local battle   = require("brain.policies.battle")
local danger   = require("brain.policies.danger")
local escort   = require("brain.policies.escort")
local recovery = require("brain.policies.recovery")
local tree     = require("brain.tree")
local escape   = require("brain.tasks.escape")

local choose, when, act = tree.select, tree.when, tree.act

local M = {}

local function flee(cause, mob_of, chance_of)
    return act("ESCAPE", function(snap, facts)
        return escape.flee(snap, facts, mob_of(snap, facts), cause)
    end, cause, function(snap, facts)
        return escape.worth(snap, facts, mob_of(snap, facts), chance_of(snap, facts), cause)
    end)
end

local function certain()
    return 1
end

-- Any member's flight makes the rest leave too: the ground was chosen for the whole group, so whoever
-- stays is alone on it. This used to apply only on floors of the current plan, so a group already
-- walking out toward a newly planned ground (trio rooted, 754s) let the fleer go alone while the other
-- two stayed. Whether the flight concerns me is decided by crew_notice (same ground, after my entry).
local function crew_notified(snap, _facts)
    return escape.crew_notice(snap) ~= nil
end

local function notice_mob(snap)
    return { name = escape.crew_notice(snap).name }
end

local function not_fleeing_myself(snap)
    return escape.fleeing_here(snap) == nil
end

local function can_walk_now(snap)
    local blocked = snap.record.memory.blocked
    return type(blocked) ~= "table" or blocked.map ~= snap.world.map or
        snap.now - blocked.at >= require("brain.policies.combat").cast(snap)
end

M.notified = when(escape.able, when(crew_notified, when(not_fleeing_myself, when(can_walk_now,
    flee("CREW", notice_mob, certain)))), "notified")

local function fatal_mob(snap, facts)
    return danger.fatal(snap, facts)
end

local function fatal_near(snap, facts)
    return fatal_mob(snap, facts) ~= nil
end

local function unrecoverable(snap, facts)
    return escape.recoverable(snap, facts, fatal_mob(snap, facts)) == false
end

local function off_plan(snap, facts)
    return escape.planned(snap, facts, fatal_mob(snap, facts)) == false
end

local function not_passing(snap, facts)
    return escape.passing(snap, facts, fatal_mob(snap, facts)) == false
end

local function fatal_chance(snap, facts)
    return danger.chance(snap, facts, fatal_mob(snap, facts), math.huge)
end

local function struck(snap)
    return battle.pressed(snap)
end

local function burst_lethal(snap)
    local burst = battle.burst(snap)
    return burst > 0 and snap.self.hp <= burst
end

local function cannot_lift(snap)
    return escape.can_lift(snap) == false
end

local function no_healer_near(snap, facts)
    local crew = escort.present(snap, facts)
    return crew == nil or crew.heal <= 0 or crew.stock <= 0
end

local function hitter_near(snap)
    return escape.adjacent_hitter(snap) ~= nil
end

local function hemmed_in(snap, facts)
    return escape.hemmed(snap, facts) ~= nil
end

local function doom(snap, facts)
    local burst = battle.burst(snap)
    if burst <= 0 then
        return 0
    end
    local pick = battle.choose(snap, facts, escape.adjacent_hitter(snap), true)
    return (pick ~= nil and pick.safest or 0) * math.max(0, math.min(1, (2 * burst - snap.self.hp) / burst))
end

local function doomed(snap, facts)
    return doom(snap, facts) > 0
end

local function below_outrun(snap, facts)
    return snap.self.hp <= escape.outrun_need(snap, facts)
end

local function stranded(snap)
    return recovery.stranded(snap)
end

M.flee = when(escape.able, choose({
    when(function(snap)
        return escape.fleeing_here(snap) ~= nil
    end, flee("FLEEING", function(snap)
        return { name = escape.fleeing_here(snap).name }
    end, certain)),
    when(crew_notified, flee("CREW", notice_mob, certain)),
    when(fatal_near, when(unrecoverable, when(off_plan, when(not_passing, flee("FATAL", fatal_mob, fatal_chance))))),
    when(struck, when(hitter_near, when(no_healer_near, choose({
        when(burst_lethal, when(cannot_lift, flee("IMMINENT", escape.adjacent_hitter, certain))),
        when(doomed, flee("DOOMED", escape.adjacent_hitter, doom)),
        when(stranded, when(below_outrun, flee("STRANDED", escape.adjacent_hitter, certain))),
    })))),
    when(hemmed_in, flee("HEMMED", escape.hemmed, certain)),
}), "flee")

return M
