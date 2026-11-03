local items = require("brain.data.items")
local shops = require("brain.data.shops")
local magic = require("brain.data.spells")
local bag   = require("brain.policies.bag")
local fight = require("brain.policies.combat")

local M = {}

local NEUTRAL = { defense = 0, ward = 0, resists = {}, size = "SMALL" }

function M.restored(snap, effect, kind)
    if effect[kind .. "_full"] then
        return snap.self["max_" .. kind] or 0
    end
    return (effect[kind .. "_ratio"] or 0) * (snap.self["max_" .. kind] or 0) + (effect[kind] or 0)
end

function M.doses(snap, kind)
    local found = {}
    for name, count in pairs(bag.usable(snap)) do
        local effect = items[name]
        if effect ~= nil and count > 0 then
            local amount = M.restored(snap, effect, kind)
            if amount > 0 then
                table.insert(found, { name = name, amount = amount, count = count })
            end
        end
    end
    table.sort(found, function(left, right)
        return left.amount < right.amount
    end)
    return found
end

function M.heal(snap)
    local slots = {}
    for _, learned in ipairs(snap.spells or {}) do
        slots[learned.name] = learned.slot
    end
    local found = nil
    for _, choice in ipairs(fight.options(snap, NEUTRAL).heal) do
        if choice.hp > 0 and (found == nil or choice.hp > found.hp) then
            found = { slot = slots[choice.name], hp = choice.hp, mp = choice.mp }
        end
    end
    return found
end

function M.least(snap)
    local least = nil
    for _, choice in ipairs(fight.options(snap, NEUTRAL).offense) do
        if choice.damage > 0 and choice.mp > 0 then
            least = math.min(least or math.huge, choice.mp)
        end
    end
    return least
end

function M.spells(snap)
    local slots = {}
    for _, learned in ipairs(snap.spells or {}) do
        slots[learned.name] = learned.slot
    end
    local found = {}
    for name, slot in pairs(slots) do
        local data = magic.spells[name]
        if data ~= nil and data.role == "CHARGE" then
            found.charge = { slot = slot, mp = data.mp or 0 }
        elseif data ~= nil and data.trigger == "MANA" and data.surge ~= nil then
            found.surge = { slot = slot, mp = data.mp or 0, name = name }
        end
    end
    return found
end

function M.rates(snap)
    local buffs = fight.buffs(snap)
    local max   = snap.self.max_mp or 0
    local mp    = 0
    if buffs.charge ~= nil then
        mp = math.max(mp, buffs.charge.chance * max * 1000 / fight.cast(snap))
    end
    if buffs.surge ~= nil then
        mp = math.max(mp, buffs.surge.rate * max)
    end
    local hp   = 0
    local heal = M.heal(snap)
    if heal ~= nil and heal.mp <= 0 then
        hp = heal.hp * 1000 / fight.cast(snap)
    elseif heal ~= nil then
        hp = heal.hp * math.min(1000 / fight.cast(snap), mp / heal.mp)
    end
    return { hp = hp, mp = mp }
end

function M.sustains(snap)
    return M.rates(snap).mp > 0 or #M.doses(snap, "mp") > 0
end

function M.potable(snap)
    if M.rates(snap).mp > 0 then
        return false
    end
    local options = fight.options(snap, NEUTRAL)
    local needed  = false
    for _, kind in ipairs({ "offense", "strike" }) do
        for _, choice in ipairs(options[kind]) do
            needed = needed or (choice.damage > 0 and choice.mp > 0)
        end
    end
    if needed == false then
        return false
    end
    for name, shop in pairs(shops) do
        local effect = items[name]
        if effect ~= nil and M.restored(snap, effect, "mp") > 0 and shop.price <= (snap.self.money or 0) then
            return true
        end
    end
    return false
end

function M.reserve(snap)
    return M.sustains(snap) and "REFILL" or "DRY"
end

return M
