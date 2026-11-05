local data     = require("brain.data.combat")
local magic    = require("brain.data.spells")
local combat   = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local escort   = require("brain.policies.escort")
local party    = require("brain.policies.party")
local recovery = require("brain.policies.recovery")
local value    = require("brain.policies.value")

local M = {}

local plans = setmetatable({}, { __mode = "k" })

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

local function targets(snap)
    local found = {}
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.oid ~= nil then
            found[mob.oid] = mob
        end
    end
    return found
end

local function price(member, mana, worth)
    local mend = recovery.heal(member)
    if mend ~= nil and mend.hp > 0 then
        return mend.mp / mend.hp * mana
    end
    local regen = recovery.rates(member).hp + data.natural.hp * math.max(member.self.max_hp or 0, 1)
    return regen > 0 and worth.value / regen or math.huge
end

local function pressure(snap, member, window)
    local total = 0
    if member.world.map ~= snap.world.map or member.world.x == nil then
        return total
    end
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and combat.hits(mob) and (mob.speed or 0) > 0 and
            math.abs(mob.x - member.world.x) + math.abs(mob.y - member.world.y) == 1 then
            local _, high = combat.spread(member, mob)
            total = total + high * window / mob.speed
        end
    end
    return total
end

local function rescue(snap, query, member, current, restored, heal, worth)
    if heal.hp <= 0 then
        return 0
    end
    local missing = (member.self.max_hp or 0) - current
    local blows   = pressure(snap, member, missing / heal.hp * combat.cast(snap))
    local before  = math.min(1, blows / math.max(current, 1))
    local after   = math.min(1, blows / math.max(current + restored, 1))
    return (before - after) * danger.loss(member, query, worth)
end

local function heals(snap, query, slots, worth)
    local found   = {}
    local loss    = danger.loss(snap, query, worth)
    local mana    = value.mana(snap, query)
    local reserve = recovery.spare(snap)
    local seen    = targets(snap)
    for _, member in ipairs(party.members(snap)) do
        local ally    = query.ally(member.self.name)
        local current = ally ~= nil and ally.percent ~= nil and (member.self.max_hp or 0) * ally.percent / 100 or
            (member.self.hp or 0)
        local deficit = (member.self.max_hp or 0) - current
        local memory  = snap.record.memory.battle
        if ally ~= nil and deficit > 0 and member.self.ghost == false then
            for _, heal in ipairs(combat.options(snap, {}).heal) do
                local refused = memory ~= nil and memory.oid == ally.oid and memory.refused == heal.name
                if heal.others and slots[heal.name] ~= nil and heal.mp + reserve <= (snap.self.mp or 0) and refused == false then
                    local restored = math.min(deficit, heal.hp)
                    local wasted   = math.min(heal.hp, member.self.max_hp or 0) - restored
                    local shared   = member.battle
                    local engaged  = type(shared) == "table" and shared.map == snap.world.map and seen[shared.oid] ~= nil
                    local hunt     = party.plan(snap, query)
                    local fuel     = hunt ~= nil and (hunt.pain or 0) > 0 and hunt.rate / hunt.pain or 0
                    local saved    = engaged and (restored - wasted) * ((loss / math.max(member.self.max_hp, 1)) + fuel) or
                        (restored - wasted) * price(member, mana, worth)
                    table.insert(found, {
                        oid = ally.oid,
                        slot = slots[heal.name],
                        key = heal.name,
                        cost = heal.mp,
                        value = saved + rescue(snap, query, member, current, restored, heal, worth) - heal.mp * mana,
                    })
                end
            end
        end
    end
    local deficit = snap.self.max_hp - snap.self.hp
    if #found > 0 and deficit > 0 then
        for _, heal in ipairs(combat.options(snap, {}).heal) do
            if slots[heal.name] ~= nil and heal.mp + reserve <= (snap.self.mp or 0) then
                local restored = math.min(deficit, heal.hp)
                local share    = (2 * restored - math.min(heal.hp, snap.self.max_hp)) / math.max(snap.self.max_hp, 1)
                table.insert(found, {
                    self = true,
                    slot = slots[heal.name],
                    key = heal.name,
                    cost = heal.mp,
                    value = share * loss - heal.mp * mana,
                })
            end
        end
    end
    return found
end

local function applied(snap, oid)
    local names  = {}
    local memory = snap.record.memory.battle
    if memory ~= nil and memory.oid == oid and memory.map == snap.world.map then
        for name, at in pairs(memory.applied or {}) do
            if combat.lasting(name, at, snap.now) then
                names[name] = true
            end
        end
    end
    for _, profile in ipairs(snap.party or {}) do
        local shared = profile.battle
        if type(shared) == "table" and shared.oid == oid and shared.map == snap.world.map then
            for name, at in pairs(shared.applied or {}) do
                if combat.lasting(name, at, snap.now) then
                    names[name] = true
                end
            end
        end
    end
    return names
end

local function debuffs(snap, query, slots, worth)
    local found  = {}
    local mana   = value.mana(snap, query)
    local seen   = targets(snap)
    local helper = escort.crew({ snap }, query)
    for _, member in ipairs(party.members(snap)) do
        local shared = member.battle
        local mob    = type(shared) == "table" and shared.map == snap.world.map and seen[shared.oid] or nil
        if mob ~= nil and member.self.ghost == false then
            local names   = applied(snap, mob.oid)
            local context = escort.context(member, query, mob, helper, nil, 0)
            local before  = combat.outcome(member, query, copy(mob, { applied = names }), nil, context.aid)
            for name, slot in pairs(slots) do
                local spell = magic.spells[name]
                local cost  = spell ~= nil and spell.mp or nil
                if spell ~= nil and spell.role == "DEBUFF" and names[name] == nil and type(cost or 0) == "number" and
                    (cost or 0) <= recovery.free(snap) then
                    local after = combat.outcome(member, query, copy(mob, { applied = copy(names, { [name] = true }) }),
                        nil, context.aid)
                    local saved = before.time < math.huge and math.max(0, before.time - after.time) or 0
                    if saved > 0 then
                        table.insert(found, {
                            oid = mob.oid,
                            slot = slot,
                            key = name,
                            debuff = name,
                            cost = cost or 0,
                            value = saved / 1000 * worth.value - (cost or 0) * mana,
                        })
                    end
                end
            end
        end
    end
    return found
end

local function pending(snap, member, name, spell)
    local at = (snap.record.memory.warded or {})[member.self.name .. "|" .. name]
    return at ~= nil and snap.now - at < (spell.buff_time or 0) * 1000 and (member.now or 0) <= at
end

function M.ward(snap, query, target, name, worth)
    local spell  = magic.spells[name]
    local mana   = value.mana(snap, query)
    local plan   = party.plan(snap, query)
    local pain   = plan ~= nil and plan.pain or 0
    local derate = ((spell.boost or {}).damage_derate or 0) / 1000
    local share  = derate / (1 + derate)
    local armor  = (spell.boost or {}).buff_phydef or 0
    if armor > 0 then
        local defense = target.self.phydef or 0
        share = 1 - combat.mitigate(100, defense - armor, 0) / math.max(combat.mitigate(100, defense, 0), 1)
    end
    local saved = math.max(pain, 1) * (spell.buff_time or 0) * share
    local cost  = (spell.mp or 0) * mana
    return math.max(saved * price(target, mana, worth), share * danger.loss(target, query, worth)) - cost
end

local function wards(snap, query, slots, worth)
    local found = {}
    for _, member in ipairs(party.members(snap)) do
        local ally = query.ally(member.self.name)
        if ally ~= nil and member.self.ghost == false then
            for name, slot in pairs(slots) do
                local spell = magic.spells[name]
                if combat.warding(name) and combat.warded(member.buffs, name) == false and
                    (spell.mp or 0) <= recovery.free(snap) and
                    pending(snap, member, name, spell) == false then
                    table.insert(found, {
                        oid = ally.oid,
                        slot = slot,
                        key = name,
                        ward = member.self.name .. "|" .. name,
                        cost = spell.mp or 0,
                        value = M.ward(snap, query, member, name, worth),
                    })
                end
            end
        end
    end
    return found
end

local function plan(snap, query)
    if plans[snap] ~= nil then
        return plans[snap] or nil
    end
    local best = nil
    if snap.self.ghost == false and #party.members(snap) > 0 then
        local slots = {}
        for _, learned in ipairs(snap.spells or {}) do
            slots[learned.name] = learned.slot
        end
        local worth = value.baseline(snap, query)
        for _, group in ipairs({ heals(snap, query, slots, worth), debuffs(snap, query, slots, worth),
            wards(snap, query, slots, worth) }) do
            for _, candidate in ipairs(group) do
                if best == nil or candidate.value > best.value then
                    best = candidate
                end
            end
        end
    end
    plans[snap] = best or false
    return best
end

function M.wanted(snap, query)
    return plan(snap, query) ~= nil
end

function M.value(snap, query)
    return plan(snap, query).value
end

function M.propose(snap, query)
    local found = plan(snap, query)
    if found.self then
        return { kind = ACTION.CAST, slot = found.slot }
    end
    return {
        kind     = ACTION.ATTACK,
        oid      = found.oid,
        key      = found.key,
        ward     = found.ward,
        debuff   = found.debuff,
        maneuver = { kind = MANEUVER.CAST, slot = found.slot, self = false, cost = found.cost },
    }
end

return M
