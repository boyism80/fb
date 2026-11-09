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
    local blows = pressure(snap, member, 2 * combat.cast(snap))
    if blows <= 0 then
        return 0
    end
    return (math.exp(-current / blows) - math.exp(-(current + restored) / blows)) * danger.loss(member, query, worth)
end

local function convertible(member, mob)
    local best = 0
    for _, choice in ipairs(combat.options(member, mob).strike) do
        if (choice.hp or 0) > 0 and (choice.damage or 0) > 0 then
            best = math.max(best, choice.damage / choice.hp)
        end
    end
    return best * combat.gain(mob, member) / math.max(mob.hp or 1, 1)
end

local function volley(snap, member)
    local attackers = {}
    for _, mob in ipairs(member.world.map == snap.world.map and member.world.x ~= nil and snap.world.mobs or {}) do
        if mob.x ~= nil and combat.hits(mob) and
            math.abs(mob.x - member.world.x) + math.abs(mob.y - member.world.y) == 1 then
            table.insert(attackers, { mob = mob, count = 1 })
        end
    end
    return #attackers > 0 and combat.imminent(member, attackers) or 0
end

local function heals(snap, query, slots, worth)
    local found   = {}
    local mana    = value.spend(snap, query, nil)
    local reserve = recovery.spare(snap)
    local seen    = targets(snap)
    for _, member in ipairs(party.members(snap)) do
        local ally = query.ally(member.self.name)
        if ally == nil and member.self.ghost == false and member.world.map == snap.world.map and member.world.x ~= nil and
            (member.self.hp or 0) < (member.self.max_hp or 0) then
            local best = 0
            for _, heal in ipairs(combat.options(snap, {}).heal) do
                if heal.others and slots[heal.name] ~= nil then
                    best = math.max(best, math.min(heal.hp, member.self.max_hp - member.self.hp))
                end
            end
            local steps = math.abs(member.world.x - snap.world.x) + math.abs(member.world.y - snap.world.y)
            if best > 0 and steps > 0 then
                local hunted = member.battle
                local target = type(hunted) == "table" and seen[hunted.oid] or nil
                local fuel   = target ~= nil and convertible(member, target) or 0
                table.insert(found, {
                    urgent = pressure(snap, member, combat.cast(snap)) >= member.self.hp,
                    walk = { x = member.world.x, y = member.world.y },
                    value = best * fuel + rescue(snap, query, member, member.self.hp, best, { hp = best }, worth) -
                        steps * (snap.pace.step or data.walk) / 1000 * worth.value,
                })
            end
        end
        local current = ally ~= nil and ally.percent ~= nil and (member.self.max_hp or 0) * ally.percent / 100 or
            (member.self.hp or 0)
        local deficit = (member.self.max_hp or 0) - current
        local memory  = snap.record.memory.battle
        local dying   = 2 * volley(snap, member) >= current
        if ally ~= nil and deficit > 0 and member.self.ghost == false then
            for _, heal in ipairs(combat.options(snap, {}).heal) do
                local refused = memory ~= nil and memory.oid == ally.oid and memory.refused == heal.name
                if heal.others and slots[heal.name] ~= nil and heal.mp + (dying and 0 or reserve) <= (snap.self.mp or 0) and refused == false then
                    local restored = math.min(deficit, heal.hp)
                    local wasted   = math.min(heal.hp, member.self.max_hp or 0) - restored
                    local shared   = member.battle
                    local engaged  = type(shared) == "table" and shared.map == snap.world.map and seen[shared.oid] ~= nil
                    local fuel     = engaged and convertible(member, seen[shared.oid]) or 0
                    local saved    = engaged and restored * fuel or
                        (restored - wasted) * price(member, mana, worth)
                    table.insert(found, {
                        urgent = dying,
                        dies = current / math.max(pressure(snap, member, 1000), 1e-9),
                        oid = ally.oid,
                        slot = slots[heal.name],
                        key = heal.name,
                        cost = heal.mp,
                        value = saved + rescue(snap, query, member, current, restored, heal, worth) -
                            heal.mp * value.spend(snap, query, engaged and seen[shared.oid] or nil),
                    })
                end
            end
        end
    end
    local deficit    = snap.self.max_hp - snap.self.hp
    local mine       = snap.self.hp / math.max(pressure(snap, snap, 1000), 1e-9)
    local worthwhile = {}
    for _, heal in ipairs(combat.options(snap, {}).heal) do
        if slots[heal.name] ~= nil and heal.mp <= (snap.self.mp or 0) and heal.hp <= deficit and
            heal.hp > pressure(snap, snap, combat.cast(snap)) then
            table.insert(worthwhile, heal)
        end
    end
    if #worthwhile > 0 and pressure(snap, snap, 2 * combat.cast(snap)) >= snap.self.hp then
        local kept = {}
        for _, entry in ipairs(found) do
            if entry.dies == nil or entry.dies < mine then
                table.insert(kept, entry)
            end
        end
        if #kept < #found then
            local top = 0
            for _, entry in ipairs(found) do
                top = math.max(top, entry.value)
            end
            found = kept
            for _, heal in ipairs(worthwhile) do
                table.insert(found, {
                    self = true,
                    urgent = true,
                    slot = slots[heal.name],
                    key = heal.name,
                    cost = heal.mp,
                    value = top + heal.hp,
                })
            end
            return found
        end
    end
    if #found > 0 and deficit > 0 then
        local crew    = escort.present(snap, query)
        local helped  = (crew ~= nil and crew.heal or 0) * combat.cast(snap) / 1000
        local shared  = (snap.record.memory.battle or {}).oid
        local engaged = shared ~= nil and seen[shared] or nil
        local fuel    = engaged ~= nil and convertible(snap, engaged) or 0
        for _, heal in ipairs(combat.options(snap, {}).heal) do
            if slots[heal.name] ~= nil and heal.mp + reserve <= (snap.self.mp or 0) then
                local restored = math.max(0, math.min(deficit, heal.hp) - helped)
                local saved    = engaged ~= nil and restored * fuel or restored * price(snap, mana, worth)
                table.insert(found, {
                    self = true,
                    slot = slots[heal.name],
                    key = heal.name,
                    cost = heal.mp,
                    value = saved + rescue(snap, query, snap, snap.self.hp, restored, heal, worth) -
                        heal.mp * value.spend(snap, query, engaged),
                })
            end
        end
    end
    return found
end

local function applied(snap, oid)
    local names  = combat.marks(snap, oid)
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
        local list   = {}
        local chosen = type(shared) == "table" and shared.map == snap.world.map and seen[shared.oid] or nil
        if chosen ~= nil then
            table.insert(list, chosen)
        end
        if member.world.map == snap.world.map and member.world.x ~= nil then
            for _, near in ipairs(snap.world.mobs or {}) do
                if near ~= chosen and near.x ~= nil and combat.hits(near) and
                    math.abs(near.x - member.world.x) + math.abs(near.y - member.world.y) == 1 then
                    table.insert(list, near)
                end
            end
        end
        for _, mob in ipairs(member.self.ghost == false and list or {}) do
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
                    local saved = before.time < math.huge and
                        math.max(0, before.time - after.time) / math.max(before.time, 1) or 0
                    if saved > 0 then
                        table.insert(found, {
                            oid = mob.oid,
                            slot = slot,
                            key = name,
                            debuff = name,
                            cost = cost or 0,
                            value = saved * combat.gain(mob, member) * (mob.ratio or 1) -
                                (cost or 0) * value.spend(snap, query, mob),
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

function M.ward(snap, query, target, name, worth, current)
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
    local blows = pressure(snap, target, 2 * combat.cast(snap))
    local hp    = math.max(current or target.self.hp or 0, 1)
    local kept  = math.exp(-blows / hp)
    return math.max(saved * price(target, mana, worth), share * danger.loss(target, query, worth)) * kept - cost
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
                        value = M.ward(snap, query, member, name, worth,
                            (member.self.max_hp or 0) * (ally.percent or 100) / 100),
                    })
                end
            end
        end
    end
    return found
end

local function listed(snap, query)
    if plans[snap] ~= nil then
        return plans[snap]
    end
    local all = {}
    if snap.self.ghost == false and #party.members(snap) > 0 then
        local slots = {}
        for _, learned in ipairs(snap.spells or {}) do
            slots[learned.name] = learned.slot
        end
        local worth = value.baseline(snap, query)
        for group, candidates in pairs({ heal = heals(snap, query, slots, worth), debuff = debuffs(snap, query, slots, worth),
            ward = wards(snap, query, slots, worth) }) do
            for _, candidate in ipairs(candidates) do
                candidate.group = group
                table.insert(all, candidate)
            end
        end
    end
    plans[snap] = all
    return all
end

local function best_of(snap, query, accept)
    local best = nil
    for _, candidate in ipairs(listed(snap, query)) do
        if accept(candidate) and (best == nil or candidate.value > best.value) then
            best = candidate
        end
    end
    return best
end

local function action_of(snap, found)
    if found.walk ~= nil then
        return {
            kind = ACTION.MOVE,
            dest = { map = snap.world.map, x = found.walk.x, y = found.walk.y, range = math.min(data.sight.x, data.sight.y) },
        }
    end
    if found.self then
        return { kind = ACTION.CAST, slot = found.slot, key = found.key }
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

local function urgent(candidate)
    return candidate.urgent == true
end

local function gainful(candidate)
    return candidate.value > 0
end

function M.urgent(snap, query)
    local found = best_of(snap, query, urgent)
    return found ~= nil and action_of(snap, found) or nil
end

function M.offer(group)
    return function(snap, query)
        local found = best_of(snap, query, function(candidate)
            return candidate.group == group and gainful(candidate)
        end)
        return found ~= nil and action_of(snap, found) or nil
    end
end

function M.worth(group)
    return function(snap, query)
        local found = best_of(snap, query, function(candidate)
            return group == "urgent" and urgent(candidate) or (candidate.group == group and gainful(candidate))
        end)
        return found ~= nil and found.value or nil
    end
end

function M.wanted(snap, query)
    return best_of(snap, query, gainful) ~= nil
end

function M.value(snap, query)
    return best_of(snap, query, gainful).value
end

return M
