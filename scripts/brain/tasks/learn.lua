local buyers   = require("brain.data.buyers")
local data     = require("brain.data.combat")
local job      = require("brain.data.job")
local learning = require("brain.data.learning")
local outfits  = require("brain.data.outfits")
local shops    = require("brain.data.shops")
local combat   = require("brain.policies.combat")
local ground   = require("brain.policies.ground")
local party    = require("brain.policies.party")
local lessons  = require("brain.policies.lessons")
local memo     = require("brain.policies.memo")
local near     = require("brain.policies.near")
local rest     = require("brain.tasks.rest")
local score    = require("brain.policies.score")
local value    = require("brain.policies.value")

local M = {}

local LEARN_MENU = 2

local plans   = setmetatable({}, { __mode = "k" })
local taughts = {}

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
        for name in pairs(snap.record.memory.untaught or {}) do
            known[name] = true
        end
        local worth = value.baseline(snap, query)
        local rate  = 0
        for _, entry in ipairs(ground.ranked(snap, query)) do
            if ground.reachable(snap, query, entry.candidate) then
                rate = math.max(rate, entry.rate)
            end
        end
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
                        local reached = memo.peek(taughts, snap.self.name, score.signature(snap, ""), name)
                        if reached == nil and ground.affordable() then
                            reached = memo.get(taughts, snap.self.name, score.signature(snap, ""), name, function()
                                local after = ground.choose(lessons.taught(snap, name, entry), query)
                                return after ~= nil and after.rate or 0
                            end)
                        end
                        local gain = math.max(0, (reached or 0) - rate)
                        local net  = -price * worth.exchange
                        if gain > 0 then
                            net = net + gain * (#party.members(snap) > 0 and ground.horizon(snap, gain + rate) or
                                ground.lifetime(snap, gain + rate))
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
                                fee = entry.money,
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
            local source = shops[item] or outfits[item]
            local limit  = math.max(source.capacity or 1, 1)
            local target = limit > 1 and math.min(count, limit) or count
            if target > (held[item] or 0) then
                best.bought[item] = target - (held[item] or 0)
                price             = price + best.bought[item] * source.price
                kinds             = kinds + 1
            end
        end
        if price + best.fee > (snap.self.money or 0) or kinds > (snap.bag.capacity or 0) - (snap.bag.used or #snap.bag.items) then
            best.bought = best.own
        end
    end
    if data.journal and best ~= nil then
        log("debug",
            string.format("[brain:learn] bot=%s t=%s map=%s skill=%s net=%.0f gain=%.0f span=%.0f price=%s",
                snap.self.name or "", tostring(snap.now),
                tostring(snap.world.map), tostring(best.name), best.value, best.gain, best.span, tostring(best.price)))
    end
    plans[snap] = best or false
    return best
end

function M.wanted(snap, query)
    local found = plan(snap, query)
    if found == nil or (#party.members(snap) > 0 and rest.low(snap, query)) then
        return false
    end
    local worth = value.baseline(snap, query)
    local trip  = value.cost(snap, query, M.propose(snap, query), worth)
    return found.value - trip * (1 + #party.members(snap)) > 0
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
    return first, source.capacity and source.capacity > 1 and found.bought[first] or 1, source.npcs
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
