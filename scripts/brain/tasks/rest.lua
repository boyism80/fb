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

local function low(snap, query)
    local threat = danger.threat(snap, ahead(snap, query))
    local plan   = party.plan(snap, query)
    local crew   = escort.present(snap, query)
    local hp     = snap.self.hp + (crew ~= nil and crew.heal or 0) * combat.cast(snap) / 1000
    return snap.self.hp < snap.self.max_hp and
        ((snap.record.death == nil and snap.self.hp <= data.shrine.limit) or (threat > 0 and hp <= threat) or
            hp <= battle.reachable(snap) or hp <= (plan ~= nil and plan.peak or 0) or
            hp <= ground.need(snap, query) or hp <= battle.need(snap, query))
end

M.low = low

local function dose(doses, deficit)
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

local function shrine(snap, query)
    local spot, move = near.approach(snap, query, data.shrine.npcs)
    return spot ~= nil and thanks(snap) or move
end

function M.thank(snap, query)
    if snap.self.ghost or snap.self.hp > data.shrine.limit or snap.self.hp >= snap.self.max_hp then
        return nil
    end
    return near.approach(snap, query, data.shrine.npcs) ~= nil and thanks(snap) or nil
end

local price

local healer

local function edible(snap, query)
    local worth = value.baseline(snap, query)
    local hurt  = 0
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and combat.hits(mob) and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
            local _, high = combat.spread(snap, mob)
            hurt = hurt + high * combat.cast(snap) / math.max(mob.speed or 1, 1)
        end
    end
    local found = {}
    if hurt <= 0 and healer(snap, query) ~= nil then
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

healer = function(snap, query)
    local plan = party.plan(snap, query)
    for _, member in ipairs(party.members(snap)) do
        local heal   = recovery.heal(member)
        local hunted = false
        for _, floor in ipairs(plan ~= nil and plan.floors or {}) do
            hunted = hunted or floor.map == member.world.map
        end
        if member.self.ghost == false and member.world.map ~= snap.world.map and hunted == false and
            heal ~= nil and heal.hp > 0 and heal.mp <= (member.self.mp or 0) then
            return { kind = ACTION.MOVE, dest = { maps = { member.world.map } } }
        end
    end
    return nil
end

local function mend(snap, query)
    if low(snap, query) == false then
        return nil
    end
    local heal = recovery.mending(snap, snap.self.max_hp - snap.self.hp)
    local crew = escort.present(snap, query)
    local able = false
    for _, member in ipairs(crew ~= nil and party.members(snap) or {}) do
        local spell = recovery.heal(member)
        able = able or (spell ~= nil and spell.hp > 0 and spell.mp <= (member.self.mp or 0))
    end
    if heal ~= nil and heal.hp > (able and crew.heal or 0) * combat.cast(snap) / 1000 then
        return { kind = ACTION.CAST, slot = heal.slot }
    end
    local doses = edible(snap, query)
    if #doses > 0 then
        return { kind = ACTION.USE, item = dose(doses, snap.self.max_hp - snap.self.hp) }
    end
    if snap.self.hp <= data.shrine.limit and snap.record.death == nil then
        return shrine(snap, query)
    end
    return healer(snap, query)
end

local function buffed(snap, name)
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

local function charged(snap, query, spells, buffs)
    local charge = spells.charge
    local mp     = snap.self.mp or 0
    if charge == nil or mp < charge.mp then
        return nil
    end
    local floor  = value.spend(snap, query, nil)
    local mana   = floor + math.max(0, value.mana(snap, query) - floor) * (1 - mp / math.max(snap.self.max_mp or 1, 1))
    local gain   = mana * ((snap.self.max_mp or 0) - mp + charge.mp) * buffs.charge.chance
    local cost   = mana * charge.mp + price(snap, query) * buffs.charge.cost
    local after  = copy(snap, {
        self = copy(snap.self, {
            hp = recovery.charged(snap) // 1 | 0,
            mp = (mp - charge.mp + ((snap.self.max_mp or 0) - mp) * buffs.charge.chance) // 1 | 0,
        })
    })
    local plan   = party.plan(snap, query)
    local floor  = math.max(danger.threat(snap, ahead(snap, query)), battle.burst(snap), battle.reachable(snap),
        plan ~= nil and plan.peak or 0)
    local mended = recovery.mending(after, snap.self.max_hp - after.self.hp)
    local lifted = math.min(snap.self.max_hp, after.self.hp + (mended ~= nil and mended.hp or 0))
    cost         = cost + (mended ~= nil and 2 or 1) * combat.cast(snap) / 1000 * value.baseline(snap, query).value
    if gain > cost and after.self.hp > 2 * battle.burst(snap) and lifted > floor and lifted > ground.need(snap, query) and
        lifted - battle.need(after, query) > floor then
        return { kind = ACTION.CAST, slot = charge.slot }
    end
    local deficit = snap.self.max_hp - snap.self.hp
    local heal    = recovery.mending(snap, deficit)
    if heal ~= nil and mp >= heal.mp + charge.mp and gain > cost + mana * heal.mp and deficit > 0 then
        return { kind = ACTION.CAST, slot = heal.slot }
    end
    return false
end

local function refill(snap, query)
    local least = recovery.least(snap)
    local mp    = snap.self.mp or 0
    local top   = recovery.ceiling(snap)
    if least == nil or mp >= top then
        return nil
    end
    local spells = recovery.spells(snap)
    local buffs  = combat.buffs(snap)
    local charge = charged(snap, query, spells, buffs)
    if charge ~= nil then
        return charge or nil
    end
    local plan = ground.choose(snap, query)
    local need = math.min(top, math.max(least, plan ~= nil and plan.spend or 0))
    if spells.charge ~= nil then
        need = spells.charge.mp
    end
    if mp >= need then
        return nil
    end
    if spells.surge ~= nil and mp >= spells.surge.mp and buffed(snap, spells.surge.name) == false then
        return { kind = ACTION.CAST, slot = spells.surge.slot }
    end
    local doses = recovery.doses(snap, "mp")
    if #doses > 0 then
        return { kind = ACTION.USE, item = dose(doses, top - mp) }
    end
    return nil
end

local function worth_of(snap, query, action, healing)
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
        local need    = math.max(danger.threat(snap, ahead(snap, query)), ground.need(snap, query),
            battle.need(snap, query), battle.reachable(snap), plan ~= nil and plan.peak or 0,
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
        gain = amount / need * danger.loss(snap, query, worth)
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

local chosen = setmetatable({}, { __mode = "k" })

local function choose(snap, query)
    if chosen[snap] ~= nil then
        return chosen[snap] or nil
    end
    local best = nil
    if snap.self.ghost == false then
        local rivals = {}
        for _, entry in ipairs({
            { action = M.thank(snap, query), healing = true, key = "thank" },
            { action = mend(snap, query), healing = true, key = "mend" },
            { action = refill(snap, query), healing = false, key = "refill" },
        }) do
            if entry.action ~= nil then
                entry.value = worth_of(snap, query, entry.action, entry.healing)
                table.insert(rivals, { key = entry.action.item or entry.key, value = entry.value })
                if best == nil or entry.value > best.value then
                    best = entry
                end
            end
        end
        if best ~= nil then
            best.action.rivals = rivals
            best.action.scale  = best.value
        end
    end
    chosen[snap] = best or false
    return best
end

function M.wanted(snap, query)
    return choose(snap, query) ~= nil
end

function M.value(snap, query)
    return choose(snap, query).value
end

function M.propose(snap, query)
    return choose(snap, query).action
end

return M
