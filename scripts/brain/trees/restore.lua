local data     = require("brain.data.combat")
local combat   = require("brain.policies.combat")
local recovery = require("brain.policies.recovery")
local battle   = require("brain.policies.battle")
local tree     = require("brain.tree")
local rest     = require("brain.tasks.rest")

local choose, when, act, pick = tree.select, tree.when, tree.act, tree.pick

local function healing(snap, facts, action)
    return rest.worth(snap, facts, action, true)
end

local function refueling(snap, facts, action)
    return math.max(0, rest.worth(snap, facts, action, false))
end

local function charge_kills(snap)
    return recovery.charged(snap) <= battle.burst(snap)
end

local function alive(snap)
    return snap.self.ghost == false
end

local function below_shrine_limit(snap)
    return snap.self.hp <= data.shrine.limit
end

local function wounded(snap)
    return snap.self.hp < snap.self.max_hp
end

local function near_shrine(snap, facts)
    return rest.shrine_near(snap, facts)
end

local function cure_of(snap)
    return recovery.mending(snap, snap.self.max_hp - snap.self.hp)
end

local function cure_helps(snap, facts)
    local cure = cure_of(snap)
    if cure == nil then
        return false
    end
    return cure.hp > rest.mending(snap, facts) * combat.cast(snap) / 1000
end

local function cure_beats_food(snap, facts)
    local most = 0
    for _, dose in ipairs(rest.edible(snap, facts)) do
        most = math.max(most, dose.amount)
    end
    return cure_of(snap).hp >= most
end

local function cure_leaves_prime(snap)
    return cure_of(snap).mp + recovery.prime(snap) <= recovery.free(snap)
end

local function cast_cure(snap)
    return { kind = ACTION.CAST, slot = cure_of(snap).slot }
end

local function has_charge(snap)
    return recovery.spells(snap).charge ~= nil
end

local function can_cast_mana(snap)
    return recovery.least(snap) ~= nil and (snap.self.mp or 0) < recovery.ceiling(snap)
end

local function below_charge_cost(snap)
    return (snap.self.mp or 0) < recovery.spells(snap).charge.mp
end

local function lacks_prime(snap)
    return recovery.free(snap) < recovery.prime(snap)
end

local function can_heal(snap)
    return recovery.heal(snap) ~= nil
end

local function cast_charge(snap)
    return { kind = ACTION.CAST, slot = recovery.spells(snap).charge.slot }
end

local function has_mana_dose(snap)
    return #recovery.doses(snap, "mp") > 0
end

local function drink(snap)
    return { kind = ACTION.USE, item = rest.dose(recovery.doses(snap, "mp"), recovery.ceiling(snap) - (snap.self.mp or 0)) }
end

local function under_spend(snap, facts)
    return (snap.self.mp or 0) < rest.mana_need(snap, facts)
end

local function surge_ready(snap)
    local surge = recovery.spells(snap).surge
    return surge ~= nil and (snap.self.mp or 0) >= surge.mp and rest.buffed(snap, surge.name) == false
end

local function cast_surge(snap)
    return { kind = ACTION.CAST, slot = recovery.spells(snap).surge.slot }
end

local refuel = when(can_cast_mana, choose({
    when(has_charge, choose({
        when(below_charge_cost, when(has_mana_dose, act("REST", drink, "drink", refueling))),
        when(lacks_prime, choose({
            when(charge_kills, when(has_mana_dose, act("REST", drink, "drink_under_attack", refueling))),
            when(combat.can_cast, choose({
                when(can_heal, act("REST", cast_charge, "charge_to_heal", refueling)),
                act("REST", cast_charge, "charge", refueling),
            })),
        })),

    })),
    when(function(snap)
        return has_charge(snap) == false
    end, when(under_spend, choose({
        when(combat.can_cast, when(surge_ready, act("REST", cast_surge, "surge", refueling))),
        when(has_mana_dose, act("REST", drink, "drink", refueling)),
    }))),
}), "refuel")

local function has_edible(snap, facts)
    return #rest.edible(snap, facts) > 0
end

local function eat(snap, facts)
    return { kind = ACTION.USE, item = rest.dose(rest.edible(snap, facts), snap.self.max_hp - snap.self.hp) }
end

local function no_death_record(snap)
    return snap.record.death == nil
end

local function go_shrine(snap, facts)
    return rest.go_shrine(snap, facts)
end

local function healer_beside(snap, facts)
    return rest.mending(snap, facts) > 0
end

local function await_heal()
    return { kind = ACTION.HOLD }
end

local cure   = when(combat.can_cast, when(cure_helps, when(cure_beats_food, act("REST", cast_cure, "cure", healing))))
local food   = when(has_edible, act("REST", eat, "eat", healing))
local shrine = when(below_shrine_limit, when(no_death_record, act("REST", go_shrine, "to_shrine", healing)))

-- The ways to recover are weighed against each other instead of taken in a fixed order, and the walk to the
-- shrine costs its time like anything else (job gather_offering: 135s for 6 hp). Natural regeneration is
-- never one of them: it is a bonus of the server, and waiting for it stalled bots for minutes.
local mend = pick({
    cure,
    food,
    shrine,
    -- No walking to a crewmate to be healed: outside the ground each member recovers on its own (its
    -- cure or food), and inside the group is already together.
    when(healer_beside, act("REST", await_heal, "await_heal", healing)),
}, tree.compared, "mend")

-- With nothing else left to do, the best way to recover is taken even when it would not pay on its own, since
-- the other choice is standing still: a 1 hp level-5 bot 20 tiles from the shrine stood for good because its
-- cheap death made the 5s walk look too dear (state pure 10).
local function best(_, _, found)
    return found.worth
end

local M = {}

M.recuperate = when(alive, when(rest.low, pick({ cure, food, shrine }, best, "recuperate")), "recuperate")

function M.mendable(snap, facts)
    return rest.low(snap, facts) and tree.run(mend, snap, facts) ~= nil
end

M.restore = when(alive, choose({
    when(below_shrine_limit, when(wounded, when(near_shrine, act("REST", rest.thank, "thank", healing)))),
    when(rest.low, choose({
        when(combat.can_cast,
            when(cure_helps,
                when(cure_beats_food, when(cure_leaves_prime, act("REST", cast_cure, "cure_first", healing))))),
        when(has_edible, act("REST", eat, "eat_first", healing)),
        -- A cure that would leave less mana than the strongest attack needs is still weighed against a charge
        -- first, not skipped for it: a poet keeping 597 mp for 지진 charged under three skeletons instead of
        -- healing itself, and the failed charges left it at 918 hp (WA+PO 세작 seed 0, 282-283s). Only a charge
        -- is weighed so, as it is paid in hp; weighed against a drink, a warrior under four 흑령 cast a 50 hp cure
        -- and drank 동동주 for its mana by turns instead of fighting (offline solo seed 101, 641s).
        when(combat.can_cast, when(has_charge, when(cure_helps,
            pick({ act("REST", cast_cure, "cure_over_refuel", healing), refuel }, tree.compared, "cure_or_refuel")))),
    })),
    refuel,
    when(rest.low, mend),
}), "restore")

return M
