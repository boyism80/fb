local data     = require("brain.data.combat")
local magic    = require("brain.data.spells")
local combat   = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local ground   = require("brain.policies.ground")
local near     = require("brain.policies.near")
local recovery = require("brain.policies.recovery")
local value    = require("brain.policies.value")

local M = {}

local function low(snap, query)
    local threat = danger.threat(snap)
    return (threat > 0 and snap.self.hp <= threat) or
        snap.self.hp <= math.min(snap.self.max_hp - 1, ground.need(snap, query))
end

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

local function mend(snap, query)
    if low(snap, query) == false then
        return nil
    end
    local heal = recovery.heal(snap)
    if heal ~= nil and (snap.self.mp or 0) >= heal.mp then
        return { kind = ACTION.CAST, slot = heal.slot }
    end
    local doses = recovery.doses(snap, "hp")
    if #doses > 0 then
        return { kind = ACTION.USE, item = dose(doses, snap.self.max_hp - snap.self.hp) }
    end
    if snap.self.hp <= data.shrine.limit then
        return shrine(snap, query)
    end
    return nil
end

local function buffed(snap, name)
    for _, buff in ipairs(snap.buffs or {}) do
        if buff == name then
            return true
        end
    end
    return false
end

local function refill(snap)
    local least = recovery.least(snap)
    local mp    = snap.self.mp or 0
    if least == nil or mp >= least then
        return nil
    end
    local spells = recovery.spells(snap)
    local buffs  = combat.buffs(snap)
    if spells.charge ~= nil and mp >= spells.charge.mp and snap.self.hp - buffs.charge.cost > danger.threat(snap) then
        return { kind = ACTION.CAST, slot = spells.charge.slot }
    elseif spells.surge ~= nil and mp >= spells.surge.mp and buffed(snap, spells.surge.name) == false then
        return { kind = ACTION.CAST, slot = spells.surge.slot }
    end
    local doses = recovery.doses(snap, "mp")
    if #doses > 0 then
        return { kind = ACTION.USE, item = dose(doses, (snap.self.max_mp or 0) - mp) }
    end
    return nil
end

local function choose(snap, query)
    if snap.self.ghost then
        return nil
    end
    return M.thank(snap, query) or mend(snap, query) or refill(snap)
end

function M.wanted(snap, query)
    return choose(snap, query) ~= nil
end

function M.value(snap, query)
    local action = choose(snap, query)
    local worth  = value.baseline(snap, query)
    local cast   = 1000 / magic.cast_rate
    local gain   = 0
    local time   = action.kind == ACTION.CAST and cast or action.kind == ACTION.USE and cast or 0
    if M.thank(snap, query) == nil and mend(snap, query) == nil then
        local full = ground.choose(snap, query, "REFILL")
        local dry  = ground.choose(snap, query, "DRY")
        local mp   = (snap.self.max_mp or 0) - (snap.self.mp or 0)
        if full ~= nil and full.drain > 0 then
            gain = (full.rate - (dry ~= nil and dry.rate or 0)) * mp / full.drain
        end
    else
        local need    = math.max(danger.threat(snap), ground.need(snap, query), 1)
        local deficit = math.max(0, need - snap.self.hp)
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
    return gain - value.cost(snap, query, action, worth, time)
end

function M.propose(snap, query)
    return choose(snap, query)
end

return M
