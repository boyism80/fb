local magic    = require("brain.data.spells")
local aid      = require("brain.tasks.aid")
local combat   = require("brain.policies.combat")
local recovery = require("brain.policies.recovery")
local ground   = require("brain.policies.ground")
local value    = require("brain.policies.value")

local M = {}

local MODELED = { damage_rate = true, buff_hit = true, buff_dam = true, buff_phydef = true, damage_derate = true }

local function echoed(name)
    for _, echo in ipairs(magic.echoes or {}) do
        if echo == name then
            return true
        end
    end
    return false
end

local function boosted(data)
    for key in pairs(data.boost or {}) do
        if MODELED[key] then
            return true
        end
    end
    return false
end

local function valued(name, data)
    return boosted(data) or (magic.multipliers or {})[name] ~= nil or (magic.splashes or {})[name] ~= nil or
        echoed(name)
end

local function hunting(snap, query)
    local order = snap.record.memory.order
    return (order ~= nil and order.task == "HUNT") or (snap.self.class ~= "NONE" and ground.choose(snap, query) ~= nil)
end

local function worth(snap, query, name, data)
    local spells = {}
    for _, spell in ipairs(snap.spells or {}) do
        if spell.name ~= name then
            table.insert(spells, spell)
        end
    end
    local without = setmetatable({ spells = spells }, { __index = snap })
    local power   = value.power(snap, query)
    local lost    = power > 0 and 1 - value.power(without, query) / power or 0
    local rate    = value.baseline(snap, query).value
    local cast    = combat.cast(snap) / 1000
    return rate * (lost * (data.buff_time or 0) - cast) - value.mana(snap, query) * (data.mp or 0)
end

local function wanted(snap, query)
    if snap.self.ghost or hunting(snap, query) == false then
        return nil
    end
    local active = {}
    for _, name in ipairs(snap.buffs or {}) do
        active[name] = true
    end
    local learned = {}
    for _, spell in ipairs(snap.spells or {}) do
        learned[spell.name] = spell.slot
    end
    local best = nil
    for name, slot in pairs(learned) do
        local data = magic.spells[name]
        if data ~= nil and data.role == "BUFF" and data.self ~= false and data.trigger == nil and valued(name, data) and active[name] == nil and
            recovery.free(snap) >= (data.mp or 0) then
            local blocked = false
            local rate    = (data.boost or {}).damage_rate or 0
            for _, other in ipairs(data.excludes or {}) do
                local rival = magic.spells[other]
                blocked     = blocked or active[other] ~= nil or
                    (learned[other] ~= nil and rival ~= nil and ((rival.boost or {}).damage_rate or 0) > rate)
            end
            if blocked == false then
                local gain = worth(snap, query, name, data)
                if best == nil or gain > best.value or (gain == best.value and name < best.name) then
                    best = { name = name, slot = slot, value = gain }
                end
            end
        end
    end
    local rate = value.baseline(snap, query)
    for name, slot in pairs(learned) do
        local data = magic.spells[name]
        if combat.warding(name) and combat.warded(snap.buffs, name) == false and
            recovery.free(snap) >= (data.mp or 0) then
            local gain = aid.ward(snap, query, snap, name, rate)
            if best == nil or gain > best.value or (gain == best.value and name < best.name) then
                best = { name = name, slot = slot, value = gain }
            end
        end
    end
    return best
end

function M.wanted(snap, query)
    return wanted(snap, query) ~= nil
end

function M.value(snap, query)
    return wanted(snap, query).value
end

function M.propose(snap, query)
    local choice = wanted(snap, query)
    return { kind = ACTION.CAST, slot = choice.slot, buff = choice.name }
end

return M
