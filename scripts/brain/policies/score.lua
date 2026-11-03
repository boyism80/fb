local combat   = require("brain.data.combat")
local items    = require("brain.data.items")
local magic    = require("brain.data.spells")
local teleport = require("brain.data.teleport")
local bag      = require("brain.policies.bag")
local fight    = require("brain.policies.combat")
local recovery = require("brain.policies.recovery")

local M = {}

local NEUTRAL = { defense = 0, ward = 0, resists = {}, size = "SMALL" }

local function sustained(choice, field, regen, rate)
    if choice == nil or choice[field] <= 0 then
        return 0
    end
    local paced = choice[field] * rate
    if choice.mp <= 0 then
        return paced
    end
    return math.min(paced, choice[field] * regen / choice.mp)
end

local function strongest(choices, field, accept)
    local found = nil
    for _, choice in ipairs(choices) do
        if accept(choice) and (found == nil or choice[field] > found[field]) then
            found = choice
        end
    end
    return found
end

local function supplies(snap)
    local result = { hp = 0, mp = 0, money = snap.self.money or 0 }
    for name, count in pairs(bag.usable(snap)) do
        local effect = items[name]
        if effect ~= nil then
            local hp = effect.hp_full and snap.self.max_hp or
                (effect.hp_ratio or 0) * snap.self.max_hp + (effect.hp or 0)
            local mp = effect.mp_full and snap.self.max_mp or
                (effect.mp_ratio or 0) * snap.self.max_mp + (effect.mp or 0)
            result.hp = result.hp + math.max(0, hp) * count
            result.mp = result.mp + math.max(0, mp) * count
        end
    end
    return result
end

local function mobility(snap)
    local held  = {}
    local found = {}
    for _, spell in ipairs(snap.spells or {}) do
        held[spell.name] = true
    end
    for _, item in ipairs(snap.bag.items) do
        held[item.name] = true
    end
    for _, means in ipairs(teleport.spells) do
        if held[means.name] then
            table.insert(found, means.name)
        end
    end
    for _, means in ipairs(teleport.items) do
        if held[means.name] then
            table.insert(found, means.name)
        end
    end
    return found
end

function M.rested(snap, mode)
    local mp   = (mode or recovery.reserve(snap)) == "REFILL" and snap.self.max_mp or 0
    local full = setmetatable({ hp = snap.self.max_hp, mp = mp }, { __index = snap.self })
    return setmetatable({ self = full }, { __index = snap })
end

function M.signature(snap, mode)
    local gear = {}
    for _, slot in ipairs(snap.gear) do
        table.insert(gear, table.concat(slot.names, "+"))
    end
    local active = {}
    for _, name in ipairs(snap.buffs or {}) do
        active[name] = true
    end
    local spells = {}
    local phydef = snap.self.phydef or 0
    local hit    = snap.self.hit or 0
    local dam    = snap.self.dam or 0
    for _, spell in ipairs(snap.spells or {}) do
        table.insert(spells, spell.name)
        local data = magic.spells[spell.name]
        if data ~= nil and data.role == "BUFF" and active[spell.name] then
            local stats = data.stats or {}
            phydef      = phydef - (stats.buff_phydef or 0)
            hit         = hit - (stats.buff_hit or 0)
            dam         = dam - (stats.buff_dam or 0)
        end
    end
    return table.concat({ snap.self.name or "", snap.self.level, snap.self.max_hp, snap.self.max_mp or 0, phydef, hit,
        dam, table.concat(gear, ","), table.concat(spells, ","), mode or recovery.reserve(snap) }, "|")
end

function M.capability(snap, query)
    local options = fight.options(snap, NEUTRAL)
    local refill  = recovery.rates(snap)
    local regen   = refill.mp
    local rate    = 1000 / fight.cast(snap)
    local melee   = fight.strike(snap, query, NEUTRAL) * 1000 / combat.swing
    local single  = strongest(options.offense, "damage", function(choice)
        return magic.spells[choice.name].role == "ATTACK"
    end)
    local area    = strongest(options.offense, "damage", function(choice)
        return magic.spells[choice.name].role == "AREA"
    end)
    local heal    = strongest(options.heal, "hp", function()
        return true
    end)
    local others  = strongest(options.heal, "hp", function(choice)
        return choice.others
    end)
    local opener  = strongest(options.strike, "damage", function()
        return true
    end)
    local nuke    = single ~= nil and single.mp > 0 and math.floor((snap.self.mp or 0) / single.mp) * single.damage or 0
    local guard   = fight.mitigate(100, snap.self.phydef or 0, 0) / 100

    return {
        dps      = {
            single = melee + sustained(single, "damage", regen, rate),
            area   = sustained(area, "damage", regen, rate),
            melee  = melee,
        },
        burst    = math.max(opener ~= nil and opener.damage or 0, nuke),
        ehp      = snap.self.hp / guard,
        hps      = {
            self   = sustained(heal, "hp", regen, rate),
            others = sustained(others, "hp", regen, rate),
        },
        supply   = supplies(snap),
        refill   = refill,
        mobility = mobility(snap),
    }
end

return M
