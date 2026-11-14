local magic    = require("brain.data.spells")
local aid      = require("brain.tasks.aid")
local combat   = require("brain.policies.combat")
local recovery = require("brain.policies.recovery")
local ground   = require("brain.policies.ground")
local value    = require("brain.policies.value")

local M = {}

function M.hunting(snap, query)
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
    local without  = setmetatable({ spells = spells }, { __index = snap })
    local baseline = value.baseline(snap, query)
    -- A buff that strikes the mobs beside the target is worth the crowd the ground puts beside it: weighed against
    -- one mob alone, 측면공격 and 후면공격 came out worth nothing on every ground and no warrior ever cast them
    -- (online 120559 and 123007, offline 115523).
    local extra    = baseline.hunt ~= nil and baseline.hunt.crowd or nil
    local power    = value.power(snap, query, extra)
    local lost     = power > 0 and 1 - value.power(without, query, extra) / power or 0
    local rate     = baseline.value
    local cast     = combat.cast(snap) / 1000
    return rate * (lost * (data.buff_time or 0) - cast) - value.spend(snap, query) * (data.mp or 0)
end

function M.best(snap, query)
    local active = {}
    for _, name in ipairs(snap.buffs or {}) do
        active[name] = true
    end
    local learned = {}
    for _, spell in ipairs(snap.spells or {}) do
        learned[spell.name] = spell.slot
    end
    local best    = nil
    local weighed = {}
    for name, slot in pairs(learned) do
        local data = magic.spells[name]
        if data ~= nil and data.role == "BUFF" and data.self ~= false and data.trigger == nil and combat.valued(name) and active[name] == nil and
            recovery.free(snap) >= (data.mp or 0) then
            local blocked = false
            local rate    = (data.boost or {}).damage_rate or 0
            for _, other in ipairs(data.excludes or {}) do
                local rival = magic.spells[other]
                blocked     = blocked or active[other] ~= nil or
                    (learned[other] ~= nil and rival ~= nil and ((rival.boost or {}).damage_rate or 0) > rate)
            end
            local gain = blocked == false and worth(snap, query, name, data) or nil
            weighed[name] = gain ~= nil and math.floor(gain) or "blocked"
            if gain ~= nil and (best == nil or gain > best.value or (gain == best.value and name < best.name)) then
                best = { name = name, slot = slot, value = gain }
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
    if best ~= nil then
        best.weighed = weighed
    end
    return best
end

function M.value(snap, query)
    return M.best(snap, query).value
end

function M.cast(snap, query)
    local choice = M.best(snap, query)
    return { kind = ACTION.CAST, slot = choice.slot, buff = choice.name, weighed = choice.weighed }
end

return M
