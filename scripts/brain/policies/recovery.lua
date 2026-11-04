local data  = require("brain.data.combat")
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

local dosing = setmetatable({}, { __mode = "k" })

function M.doses(snap, kind)
    local usable = bag.usable(snap)
    local known  = dosing[usable]
    if known == nil then
        known = {}
        dosing[usable] = known
    end
    local key = kind .. "|" .. tostring(snap.self.max_hp) .. "|" .. tostring(snap.self.max_mp)
    if known[key] ~= nil then
        return known[key]
    end
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
    known[key] = found
    return found
end

function M.heal(snap)
    local options = fight.options(snap, NEUTRAL)
    if options.strongest ~= nil then
        return options.strongest or nil
    end
    local slots = {}
    for _, learned in ipairs(snap.spells or {}) do
        slots[learned.name] = learned.slot
    end
    local found = nil
    for _, choice in ipairs(options.heal) do
        if choice.hp > 0 and (found == nil or choice.hp > found.hp) then
            found = { slot = slots[choice.name], hp = choice.hp, mp = choice.mp }
        end
    end
    options.strongest = found or false
    return found
end

function M.spendable(snap)
    local charge = M.spells(snap).charge
    return math.max(0, (snap.self.mp or 0) - (charge ~= nil and charge.mp or 0))
end

local freed = setmetatable({}, { __mode = "k" })

function M.free(snap)
    local known = freed[snap]
    if known ~= nil then
        return known
    end
    local left   = M.spendable(snap)
    local charge = fight.buffs(snap).charge
    local heal   = M.heal(snap)
    if charge ~= nil and heal ~= nil and heal.hp > 0 then
        local attackers = {}
        for _, other in ipairs(snap.world.mobs or {}) do
            if other.x ~= nil and fight.hits(other) and
                math.abs(other.x - snap.world.x) + math.abs(other.y - snap.world.y) == 1 then
                table.insert(attackers, { mob = other, count = 1 })
            end
        end
        local burst = #attackers > 0 and fight.spike(snap, attackers) or 0
        left = math.max(0, left - math.max(0, charge.cost + 2 * burst - snap.self.hp) / heal.hp * heal.mp)
    end
    freed[snap] = left
    return left
end

function M.mending(snap, deficit)
    local slots = {}
    for _, learned in ipairs(snap.spells or {}) do
        slots[learned.name] = learned.slot
    end
    local found = nil
    for _, choice in ipairs(fight.options(snap, NEUTRAL).heal) do
        if choice.hp > 0 and slots[choice.name] ~= nil and choice.mp <= M.spendable(snap) then
            local enough = choice.hp >= deficit
            local better = found == nil or
                (enough and (found.hp < deficit or choice.mp < found.mp)) or
                (enough == false and found.hp < deficit and choice.hp > found.hp)
            if better then
                found = { slot = slots[choice.name], hp = choice.hp, mp = choice.mp }
            end
        end
    end
    return found
end

function M.opening(snap)
    local dry   = setmetatable({ self = setmetatable({ mp = 0 }, { __index = snap.self }) }, { __index = snap })
    local least = nil
    for _, choice in ipairs(fight.options(dry, NEUTRAL).strike) do
        if choice.damage > 0 and choice.mp > 0 then
            least = math.min(least or math.huge, choice.mp)
        end
    end
    return least
end

function M.least(snap)
    local least = M.opening(snap)
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

function M.charged(snap)
    local charge = fight.buffs(snap).charge
    return math.max(charge.floor, snap.self.hp - charge.cost)
end

function M.rates(snap)
    local buffs = fight.buffs(snap)
    local max   = snap.self.max_mp or 0
    local mp    = 0
    local cast  = fight.cast(snap) / 1000
    if buffs.charge ~= nil then
        local gain    = buffs.charge.chance * max - buffs.charge.mp
        local mend    = M.heal(snap)
        local natural = data.natural.hp * math.max(snap.self.max_hp or 0, 1)
        local back    = 0
        local time    = cast
        if mend ~= nil and mend.hp > 0 then
            back = buffs.charge.cost / mend.hp * mend.mp
            time = cast * (1 + buffs.charge.cost / mend.hp)
        elseif natural > 0 then
            time = cast + buffs.charge.cost / natural
        end
        mp = math.max(mp, math.max(0, gain - back) / time)
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

local function draining(snap)
    for _, learned in ipairs(snap.spells or {}) do
        local data = magic.spells[learned.name]
        if data ~= nil and data.role == "STRIKE" and (data.damage_mp or 0) > 0 then
            return true
        end
    end
    return false
end

function M.spare(snap)
    local buffs   = fight.buffs(snap)
    local opening = M.opening(snap) or 0
    if opening > 0 and draining(snap) then
        opening = math.max(opening, snap.self.mp or 0)
    end
    if buffs.charge == nil or buffs.charge.chance <= 0 then
        return opening
    end
    local mend = M.heal(snap)
    local back = 0
    if mend ~= nil and mend.hp > 0 then
        back = buffs.charge.cost / mend.hp * mend.mp
    end
    return math.max(opening, (buffs.charge.mp + back) / buffs.charge.chance)
end

function M.ceiling(snap)
    if draining(snap) then
        return math.min(snap.self.max_mp or 0, M.opening(snap) or 0)
    end
    return snap.self.max_mp or 0
end

function M.ignition(snap)
    local buffs = fight.buffs(snap)
    if buffs.charge == nil or buffs.charge.chance <= 0 then
        return M.opening(snap) or 0
    end
    return math.max(M.opening(snap) or 0, buffs.charge.mp)
end

function M.stranded(snap)
    local heal = M.heal(snap)
    if heal ~= nil and heal.hp > 0 and heal.mp <= (snap.self.mp or 0) then
        return false
    end
    local blow = 0
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and fight.hits(mob) and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
            blow = math.max(blow, fight.blow(snap, mob))
        end
    end
    for _, dose in ipairs(M.doses(snap, "hp")) do
        if dose.amount > blow then
            return false
        end
    end
    return true
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

function M.purchasable(snap, kind)
    local best = 0
    for name, shop in pairs(shops) do
        local effect = items[name]
        if effect ~= nil and shop.price <= (snap.self.money or 0) then
            best = math.max(best, M.restored(snap, effect, kind))
        end
    end
    return best
end

function M.reserve(snap)
    return (M.sustains(snap) or M.purchasable(snap, "mp") > 0) and "REFILL" or "DRY"
end

return M
