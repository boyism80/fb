local magic  = require("brain.data.spells")
local combat = require("brain.policies.combat")
local danger = require("brain.policies.danger")
local escort = require("brain.policies.escort")
local party  = require("brain.policies.party")
local value  = require("brain.policies.value")

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

local function heals(snap, query, slots, worth)
    local found = {}
    local loss  = danger.loss(snap, query, worth)
    local mana  = value.mana(snap, query)
    for _, member in ipairs(party.members(snap)) do
        local ally    = query.ally(member.self.name)
        local deficit = (member.self.max_hp or 0) - (member.self.hp or 0)
        if ally ~= nil and deficit > 0 and member.self.ghost == false then
            for _, heal in ipairs(combat.options(snap, {}).heal) do
                if heal.others and slots[heal.name] ~= nil and heal.mp <= (snap.self.mp or 0) then
                    local share = math.min(deficit, heal.hp) / math.max(member.self.max_hp, 1)
                    table.insert(found, {
                        oid = ally.oid,
                        slot = slots[heal.name],
                        key = heal.name,
                        value = share * loss - heal.mp * mana,
                    })
                end
            end
        end
    end
    return found
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
                    (cost or 0) <= (snap.self.mp or 0) then
                    local after = combat.outcome(member, query, copy(mob, { applied = copy(names, { [name] = true }) }),
                        nil, context.aid)
                    local saved = before.time < math.huge and math.max(0, before.time - after.time) or 0
                    if saved > 0 then
                        table.insert(found, {
                            oid = mob.oid,
                            slot = slot,
                            key = name,
                            debuff = name,
                            value = saved / 1000 * worth.value - (cost or 0) * mana,
                        })
                    end
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
        for _, group in ipairs({ heals(snap, query, slots, worth), debuffs(snap, query, slots, worth) }) do
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
    return {
        kind     = ACTION.ATTACK,
        oid      = found.oid,
        key      = found.key,
        debuff   = found.debuff,
        maneuver = { kind = MANEUVER.CAST, slot = found.slot, self = false },
    }
end

return M
