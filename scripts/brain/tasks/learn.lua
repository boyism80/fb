local buyers   = require("brain.data.buyers")
local job      = require("brain.data.job")
local learning = require("brain.data.learning")
local outfits  = require("brain.data.outfits")
local shops    = require("brain.data.shops")
local combat   = require("brain.policies.combat")
local ground   = require("brain.policies.ground")
local lessons  = require("brain.policies.lessons")
local near     = require("brain.policies.near")
local value    = require("brain.policies.value")

local M = {}

local LEARN_MENU = 2

local plans = setmetatable({}, { __mode = "k" })

local function stocked(snap, query, missing)
    local kinds = 0
    for name in pairs(missing.bought) do
        local source = shops[name] or outfits[name]
        if #query.npcs(source.npcs) == 0 then
            return false
        end
        kinds = kinds + 1
    end
    return kinds <= (snap.bag.capacity or 0) - (snap.bag.used or #snap.bag.items)
end

local function affordable(snap, query, entry)
    local missing = lessons.missing(snap, entry)
    return entry.level <= snap.self.level and missing.drops == 0 and
        (snap.self.money or 0) >= entry.money + missing.price and stocked(snap, query, missing), missing
end

local function plan(snap, query)
    if plans[snap] ~= nil then
        return plans[snap] or nil
    end
    local best   = nil
    local basket = {}
    if snap.self.ghost == false and job.masters[snap.self.class] ~= nil then
        local known = {}
        for _, learned in ipairs(snap.spells or {}) do
            known[learned.name] = true
        end
        local worth = value.baseline(snap, query)
        local now   = ground.choose(snap, query)
        local rate  = now ~= nil and now.rate or 0
        for _, entry in ipairs(learning[snap.self.class] or {}) do
            local ready, missing = affordable(snap, query, entry)
            if ready then
                local price = entry.money + missing.price
                for name, count in pairs(entry.materials) do
                    price = price +
                        (count - (missing.bought[name] or 0)) * (buyers[name] ~= nil and buyers[name].price or 0)
                end
                for _, name in ipairs(entry.names) do
                    if known[name] == nil then
                        local after = ground.choose(lessons.taught(snap, name, entry), query)
                        local gain  = math.max(0, (after ~= nil and after.rate or 0) - rate)
                        local net   = -price * worth.exchange
                        if gain > 0 then
                            net = net + gain * ground.lifetime(snap, gain + rate)
                        end
                        if net > 0 then
                            for item, count in pairs(entry.materials) do
                                if shops[item] ~= nil or outfits[item] ~= nil then
                                    basket[item] = (basket[item] or 0) + count
                                end
                            end
                        end
                        if best == nil or net > best.value then
                            best = {
                                name = name,
                                value = net,
                                gain = gain,
                                own = missing.bought,
                                price = entry.money + missing.price,
                                span = ground.lifetime(snap, gain + rate)
                            }
                        end
                    end
                end
            end
        end
    end
    if best ~= nil then
        local held = {}
        for _, item in ipairs(snap.bag.items) do
            held[item.name] = (held[item.name] or 0) + item.count
        end
        local price = 0
        local kinds = 0
        best.bought = {}
        for item, count in pairs(basket) do
            if count > (held[item] or 0) then
                best.bought[item] = count - (held[item] or 0)
                price             = price + best.bought[item] * (shops[item] or outfits[item]).price
                kinds             = kinds + 1
            end
        end
        if price > (snap.self.money or 0) or kinds > (snap.bag.capacity or 0) - (snap.bag.used or #snap.bag.items) then
            best.bought = best.own
        end
    end
    plans[snap] = best or false
    return best
end

function M.wanted(snap, query)
    local found = plan(snap, query)
    if found == nil then
        return false
    end
    return found.value - value.cost(snap, query, M.propose(snap, query), value.baseline(snap, query)) > 0
end

function M.value(snap, query)
    local found = plan(snap, query)
    local net   = found.value - value.cost(snap, query, M.propose(snap, query), value.baseline(snap, query))
    return net / found.span * combat.cast(snap) / 1000
end

function M.pending(snap, query)
    local found = plan(snap, query)
    if found == nil or found.gain <= 0 or M.wanted(snap, query) == false then
        return nil
    end
    return {
        gain = found.gain,
        price = found.price,
        net = found.value,
        trip = value.cost(snap, query, M.propose(snap, query), value.baseline(snap, query))
    }
end

local function shopping(found)
    local first = nil
    for name in pairs(found.bought or {}) do
        if first == nil or name < first then
            first = name
        end
    end
    if first == nil then
        return nil
    end
    local source = shops[first] or outfits[first]
    return first, math.min(found.bought[first], math.max(1, source.capacity or 1)), source.npcs
end

function M.propose(snap, query)
    local found              = plan(snap, query)
    local item, amount, npcs = shopping(found)
    if item ~= nil then
        local spot, move = near.approach(snap, query, npcs)
        if spot ~= nil then
            return { kind = ACTION.SAY, text = string.format("%s %d개 줘", item, amount), gain = item }
        end
        return move or { kind = ACTION.HOLD }
    end
    local master     = job.masters[snap.self.class]
    local spot, move = near.approach(snap, query, { master })
    if spot ~= nil then
        return { kind = ACTION.TALK, npc = master, answers = { LEARN_MENU, found.name, "예" }, spell = found.name }
    end
    return move or { kind = ACTION.HOLD }
end

return M
