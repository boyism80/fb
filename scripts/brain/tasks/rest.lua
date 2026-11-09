local buyers   = require("brain.data.buyers")
local data     = require("brain.data.combat")
local battle   = require("brain.policies.battle")
local combat   = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local escort   = require("brain.policies.escort")
local ground   = require("brain.policies.ground")
local near     = require("brain.policies.near")
local party    = require("brain.policies.party")
local recovery = require("brain.policies.recovery")
local value    = require("brain.policies.value")

local M = {}

local function ahead(snap, query)
    local maps = { snap.world.map }
    if snap.record.death ~= nil then
        table.insert(maps, snap.record.death.map)
    end
    local dest = value.objective(snap, query).dest
    for _, map in ipairs(dest ~= nil and (dest.maps or { dest.map }) or {}) do
        table.insert(maps, map)
    end
    return maps
end

local function entry_need(snap, query)
    local charge = combat.buffs(snap).charge
    return ground.need(snap, query) + (charge ~= nil and charge.cost or 0)
end

local function outside(snap, query)
    local plan = party.plan(snap, query)
    if plan == nil then
        return false
    end
    for _, floor in ipairs(plan.floors) do
        if floor.map == snap.world.map then
            return false
        end
    end
    return true
end

-- Topping up before going in is done at the entrance, where the group gathers anyway. Doing it anywhere
-- outside the ground made a member on an errand stop for 3% of its hp and walk to a crewmate to be healed
-- (RO+PO, 680s); while walking, natural regeneration refills that much for free.
local function at_gate(snap, query, plan)
    return plan ~= nil and outside(snap, query) and
        (#party.members(snap) == 0 or snap.world.map == party.entry_gate(snap, query, plan))
end

local function low(snap, query)
    local threat = danger.threat(snap, ahead(snap, query))
    local plan   = party.plan(snap, query)
    local crew   = escort.present(snap, query)
    local hp     = snap.self.hp + escort.ready(crew, combat.cast(snap))
    return snap.self.hp < snap.self.max_hp and
        ((at_gate(snap, query, plan) and snap.self.max_hp - snap.self.hp > data.natural.hp * snap.self.max_hp) or
            (threat > 0 and hp <= threat) or
            hp <= battle.reachable(snap) or hp <= 2 * (plan ~= nil and plan.peak or 0) or
            hp <= entry_need(snap, query) or hp <= battle.need(snap, query))
end

M.outside = outside

M.low = low

function M.dose(doses, deficit)
    for _, entry in ipairs(doses) do
        if entry.amount >= deficit then
            return entry.name
        end
    end
    return doses[#doses].name
end

local function thanks(snap)
    return { kind = ACTION.SAY, text = data.shrine.words, expect = { hp = snap.self.max_hp } }
end

function M.go_shrine(snap, query)
    local spot, move = near.approach(snap, query, data.shrine.npcs)
    return spot ~= nil and thanks(snap) or move
end

function M.shrine_near(snap, query)
    return near.approach(snap, query, data.shrine.npcs) ~= nil
end

function M.thank(snap)
    return thanks(snap)
end

local price

function M.edible(snap, query)
    local worth = value.baseline(snap, query)
    local hurt  = 0
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and combat.hits(mob) and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
            local _, high = combat.spread(snap, mob)
            hurt = hurt + high * combat.cast(snap) / math.max(mob.speed or 1, 1)
        end
    end
    -- Food is skipped only when a crewmate who can heal stands beside me. Skipping it whenever a healer
    -- existed anywhere left "walk to the healer" as the only choice even for a poet that could not cast
    -- on a no-magic map (RO+PO, 680s).
    local found = {}
    local crew  = escort.present(snap, query)
    if hurt <= 0 and crew ~= nil and crew.heal > 0 then
        return found
    end
    local time = combat.cast(snap) / 1000 * worth.value
    local heal = recovery.heal(snap)
    local each = price(snap, query)
    if heal ~= nil and heal.hp > 0 then
        each = math.max(each, heal.mp / heal.hp * value.spend(snap, query, nil))
    end
    for _, entry in ipairs(recovery.doses(snap, "hp")) do
        local sale = buyers[entry.name] ~= nil and buyers[entry.name].price or 0
        if entry.amount > hurt and entry.amount * each > sale * worth.exchange + time then
            table.insert(found, entry)
        end
    end
    return found
end

function M.buffed(snap, name)
    for _, buff in ipairs(snap.buffs or {}) do
        if buff == name then
            return true
        end
    end
    return false
end

price = function(snap, query)
    local mana = value.mana(snap, query)
    local heal = recovery.heal(snap)
    if heal ~= nil and heal.hp > 0 then
        return heal.mp / heal.hp * mana
    end
    local regen = recovery.rates(snap).hp + data.natural.hp * math.max(snap.self.max_hp, 1)
    return regen > 0 and value.baseline(snap, query).value / regen or math.huge
end

local function copy(source, changes)
    local result = {}
    for key, entry in pairs(source) do
        result[key] = entry
    end
    for key, entry in pairs(changes) do
        result[key] = entry
    end
    return result
end

function M.mana_need(snap, query)
    if outside(snap, query) then
        local charge = combat.buffs(snap).charge
        local mend   = recovery.heal(snap)
        local back   = charge ~= nil and mend ~= nil and mend.hp > 0 and math.ceil(charge.cost / mend.hp) * mend.mp or 0
        return recovery.ceiling(snap) - data.natural.mp * (snap.self.max_mp or 0) - back
    end
    local plan = ground.choose(snap, query)
    return math.min(recovery.ceiling(snap), math.max(recovery.least(snap) or 0, plan ~= nil and plan.spend or 0))
end

function M.worth(snap, query, action, healing)
    local worth = value.baseline(snap, query)
    local cast  = combat.cast(snap)
    local gain  = 0
    local time  = action.kind == ACTION.CAST and cast or action.kind == ACTION.USE and cast or 0
    if healing == false then
        local deficit = math.max(0, recovery.ceiling(snap) - (snap.self.mp or 0))
        for _, entry in ipairs(action.kind == ACTION.USE and recovery.doses(snap, "mp") or {}) do
            if entry.name == action.item then
                deficit = math.min(deficit, entry.amount)
            end
        end
        gain = value.mana(snap, query) * deficit
        if low(snap, query) then
            worth = { value = 0, exchange = worth.exchange, rate = worth.rate, income = worth.income }
        end
    else
        local plan    = party.plan(snap, query)
        local need    = math.max(outside(snap, query) and snap.self.max_hp or 0, danger.threat(snap, ahead(snap, query)),
            entry_need(snap, query),
            battle.need(snap, query), battle.reachable(snap), 2 * (plan ~= nil and plan.peak or 0),
            snap.self.hp <= data.shrine.limit and data.shrine.limit + 1 or 1)
        local deficit = math.max(0, math.min(need, snap.self.max_hp) - snap.self.hp)
        local heal    = recovery.heal(snap)
        local doses   = recovery.doses(snap, "hp")
        local amount  = deficit
        if action.kind == ACTION.CAST and heal ~= nil then
            amount = math.min(deficit, heal.hp)
        elseif action.kind == ACTION.USE and #doses > 0 then
            amount = math.min(deficit, doses[#doses].amount)
        end
        local blows = battle.burst(snap)
        local saved = 0
        if blows > 0 and danger.fatal(snap, query) == nil then
            local before = math.max(0, math.min(1, (2 * blows - snap.self.hp) / blows))
            local after  = math.max(0, math.min(1, (2 * blows - snap.self.hp - amount) / blows))
            saved        = before - after
        end
        gain = math.max(amount / need, saved) * danger.loss(snap, query, worth)
        if snap.self.hp < need then
            worth = { value = 0, exchange = worth.exchange, rate = worth.rate, income = worth.income }
        end
    end
    local spent = value.cost(snap, query, action, worth, time)
    if data.journal then
        log("debug",
            string.format(
                "[brain:restval] bot=%s map=%s kind=%s item=%s gain=%.0f cost=%.0f mana=%.3f worth=%.0f hp=%s mp=%s ceiling=%s",
                snap.self.name or "", tostring(snap.world.map), tostring(action.kind), tostring(action.item), gain, spent,
                value.mana(snap, query),
                worth.value, tostring(snap.self.hp), tostring(snap.self.mp), tostring(recovery.ceiling(snap))))
    end
    return gain - spent
end

return M
