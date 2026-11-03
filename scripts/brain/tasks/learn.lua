local buyers   = require("brain.data.buyers")
local job      = require("brain.data.job")
local learning = require("brain.data.learning")
local combat   = require("brain.policies.combat")
local ground   = require("brain.policies.ground")
local near     = require("brain.policies.near")
local value    = require("brain.policies.value")

local M = {}

local LEARN_MENU = 2

local plans = setmetatable({}, { __mode = "k" })

local function holding(snap)
    local counts = {}
    for _, item in ipairs(snap.bag.items) do
        counts[item.name] = (counts[item.name] or 0) + item.count
    end
    return counts
end

local function affordable(snap, entry, counts)
    if entry.level > snap.self.level or (snap.self.money or 0) < entry.money then
        return false
    end
    for name, count in pairs(entry.materials) do
        if (counts[name] or 0) < count then
            return false
        end
    end
    return true
end

local function plan(snap, query)
    if plans[snap] ~= nil then
        return plans[snap] or nil
    end
    local best = nil
    if snap.self.ghost == false and job.masters[snap.self.class] ~= nil then
        local known  = {}
        local spells = {}
        for _, learned in ipairs(snap.spells or {}) do
            known[learned.name] = true
            table.insert(spells, learned)
        end
        local counts = holding(snap)
        local worth  = value.baseline(snap, query)
        local now    = ground.choose(snap, query)
        local rate   = now ~= nil and now.rate or 0
        for _, entry in ipairs(learning[snap.self.class] or {}) do
            if affordable(snap, entry, counts) then
                local price = entry.money
                for name, count in pairs(entry.materials) do
                    price = price + count * (buyers[name] ~= nil and buyers[name].price or 0)
                end
                for _, name in ipairs(entry.names) do
                    if known[name] == nil then
                        local taught = {}
                        for _, learned in ipairs(spells) do
                            table.insert(taught, learned)
                        end
                        table.insert(taught, { name = name, slot = 0 })
                        local after = ground.choose(
                            setmetatable({ spells = taught, hypothesis = name, actual = snap }, { __index = snap }),
                            query)
                        local gain  = math.max(0, (after ~= nil and after.rate or 0) - rate)
                        local net   = -price * worth.exchange
                        if gain > 0 then
                            net = net + gain * ground.horizon(snap, gain + rate)
                        end
                        if best == nil or net > best.value then
                            best = { name = name, value = net, gain = gain }
                        end
                    end
                end
            end
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
    return plan(snap, query).gain * combat.cast(snap) / 1000
end

function M.propose(snap, query)
    local found      = plan(snap, query)
    local master     = job.masters[snap.self.class]
    local spot, move = near.approach(snap, query, { master })
    if spot ~= nil then
        return { kind = ACTION.TALK, npc = master, answers = { LEARN_MENU, found.name, "예" } }
    end
    return move or { kind = ACTION.HOLD }
end

return M
