local magic = require("brain.data.spells")

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

local function hunting(snap)
    local order = snap.record.memory.order
    return (snap.persona or {}).hunt == 1 or (order ~= nil and order.task == "HUNT")
end

local function wanted(snap)
    if snap.self.ghost or hunting(snap) == false then
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
        if data ~= nil and data.role == "BUFF" and data.trigger == nil and valued(name, data) and active[name] == nil and
            (snap.self.mp or 0) >= (data.mp or 0) then
            local blocked = false
            local rate    = (data.boost or {}).damage_rate or 0
            for _, other in ipairs(data.excludes or {}) do
                local rival = magic.spells[other]
                blocked     = blocked or active[other] ~= nil or
                    (learned[other] ~= nil and rival ~= nil and ((rival.boost or {}).damage_rate or 0) > rate)
            end
            if blocked == false and (best == nil or name < best.name) then
                best = { name = name, slot = slot }
            end
        end
    end
    return best
end

function M.wanted(snap)
    return wanted(snap) ~= nil
end

function M.propose(snap)
    local choice = wanted(snap)
    return { kind = ACTION.CAST, slot = choice.slot, buff = choice.name }
end

return M
