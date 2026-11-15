local magic  = require("brain.data.spells")
local combat = require("brain.policies.combat")
local escort = require("brain.policies.escort")
local party  = require("brain.policies.party")
local value  = require("brain.policies.value")

local M = {}

local function guard(snap)
    for _, learned in ipairs(snap.spells or {}) do
        local data = magic.spells[learned.name]
        if data ~= nil and data.role == "BUFF" and data.trigger == "EMERGENCY" and data.self ~= false then
            return learned, data
        end
    end
    return nil, nil
end

function M.cast(snap, query)
    local learned, data = guard(snap)
    if learned == nil or combat.can_cast(snap) == false or (snap.self.mp or 0) < (data.mp or 0) or
        combat.invincible(snap) > 0 then
        return nil
    end
    return { kind = ACTION.CAST, slot = learned.slot, roll = learned.name }
end

-- A guard (금강불체) spares me every blow while it lasts, so it is worth the death it keeps off at its odds: the death
-- within two casts the mobs beside me threaten past what I and my crew hold against them, as a heal is weighed (aid
-- rescue), less the mana it spends hit or miss. A heal takes off only the part of that death its hp covers, so the
-- guard wins where heals cannot keep up, and with nothing beside me it is worth nothing.
function M.worth(snap, query, action)
    local learned, data = guard(snap)
    if learned == nil then
        return nil
    end
    local window = 2 * combat.cast(snap)
    local blows  = combat.pressure(snap, snap, window)
    if blows <= 0 then
        return -math.huge
    end
    local cover  = snap.self.hp + escort.ready(escort.present(snap, query), window)
    local risk   = math.exp(-cover / blows)
    local worth  = value.baseline(snap, query)
    local chance = combat.rolled(snap, learned.name, data.chance or 1)
    return chance * risk * party.death(snap, query, snap, worth) - (data.mp or 0) * value.spend(snap, query, nil)
end

return M
