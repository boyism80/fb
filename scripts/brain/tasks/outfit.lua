local outfits = require("brain.data.outfits")
local combat  = require("brain.policies.combat")
local gear    = require("brain.policies.gear")
local ground  = require("brain.policies.ground")
local memo    = require("brain.policies.memo")
local near    = require("brain.policies.near")
local party   = require("brain.policies.party")
local score   = require("brain.policies.score")
local value   = require("brain.policies.value")

local M = {}

local plans = setmetatable({}, { __mode = "k" })
local gains = {}
local links = {}

local function fits(snap, outfit)
    local me = snap.self
    return (outfit.level or 0) <= (me.level or 0) and (outfit.limit == nil or (me.level or 0) <= outfit.limit) and
        (outfit.class == nil or outfit.class == me.class) and (outfit.gender == nil or outfit.gender == me.gender) and
        (outfit.strength or 0) <= (me.str or 0) and (outfit.promotion or 0) <= (me.promotion or 0) and
        outfit.price <= (me.money or 0)
end

local function held(snap)
    local names = {}
    for _, item in ipairs(snap.bag.items) do
        names[item.name] = true
    end
    return names
end

local function stat(query, name, key)
    local item = name ~= nil and query.item(name) or nil
    return item ~= nil and (item[key] or 0) or 0
end

local function dressed(snap, query, kind, name)
    local slots = {}
    local old   = nil
    for _, slot in ipairs(snap.gear or {}) do
        if slot.type ~= kind or old ~= nil then
            table.insert(slots, slot)
        else
            old = slot.names[1] or false
        end
    end
    table.insert(slots, { type = kind, names = { name } })
    local was = old or nil
    local me  = setmetatable({
        phydef = (snap.self.phydef or 0) + stat(query, name, "defense") - stat(query, was, "defense"),
        max_hp = snap.self.max_hp + stat(query, name, "hp") - stat(query, was, "hp"),
        hit    = (snap.self.hit or 0) + stat(query, name, "hit") - stat(query, was, "hit"),
        dam    = (snap.self.dam or 0) + stat(query, name, "bonus") - stat(query, was, "bonus"),
    }, { __index = snap.self })
    return setmetatable({ self = me, gear = slots, hypothesis = name, actual = snap }, { __index = snap })
end

local function linked(snap, query, names)
    local goal = value.objective(snap, query).dest
    if goal == nil or goal.maps == nil then
        return names
    end
    local kept = {}
    for _, name in ipairs(names) do
        local open = true
        for _, spot in ipairs(query.npcs({ name })) do
            local key = table.concat({ name, spot.map, table.concat(goal.maps, ",") }, "|")
            if links[key] == nil then
                links[key] = query.path({ maps = goal.maps }, nil, nil, spot) ~= nil
            end
            open = open and links[key]
        end
        if open then
            table.insert(kept, name)
        end
    end
    return kept
end

local function picks(snap, query)
    local current = gear.worn(snap, query)
    local owned   = held(snap)
    for _, slot in ipairs(snap.gear or {}) do
        for _, name in ipairs(slot.names or {}) do
            if gear.score(query, name) == nil then
                current[slot.type] = nil
            end
        end
    end
    local best = {}
    for name, outfit in pairs(outfits) do
        if owned[name] == nil and fits(snap, outfit) then
            local better, rank = gear.upgrade(query, { name = name, type = outfit.type }, current)
            local found        = best[outfit.type]
            if better and rank > 0 and (found == nil or rank > found.score or (rank == found.score and name < found.name)) then
                local names = linked(snap, query, outfit.npcs)
                if #query.npcs(names) > 0 then
                    best[outfit.type] = { name = name, outfit = outfit, npcs = names, score = rank }
                end
            end
        end
    end
    return best
end

local function plan(snap, query)
    if plans[snap] ~= nil then
        return plans[snap] or nil
    end
    local best = nil
    local room = (snap.bag.capacity or 0) - (snap.bag.used or #snap.bag.items)
    if snap.self.ghost == false and snap.self.class ~= "NONE" and room > 0 then
        local worth = value.baseline(snap, query)
        local now   = ground.choose(snap, query)
        local rate  = now ~= nil and now.rate or 0
        local base  = score.signature(snap)
        for kind, pick in pairs(snap.record.death == nil and picks(snap, query) or {}) do
            local gain = memo.get(gains, snap.self.name, base, pick.name, function()
                local after = ground.choose(dressed(snap, query, kind, pick.name), query)
                return math.max(0, (after ~= nil and after.rate or 0) - rate)
            end)
            if gain > 0 then
                local span = ground.horizon(snap, gain + rate)
                local net  = gain * span - pick.outfit.price * worth.exchange
                if best == nil or net > best.value or (net == best.value and pick.name < best.name) then
                    best = { name = pick.name, npcs = pick.npcs, value = net, gain = gain, span = span }
                end
            end
        end
    end
    plans[snap] = best or false
    return best
end

local function net(snap, query)
    local found  = plan(snap, query)
    local action = M.propose(snap, query)
    if action.kind == ACTION.MOVE and #party.members(snap) > 0 then
        return -math.huge
    end
    return found.value - value.cost(snap, query, action, value.baseline(snap, query))
end

function M.wanted(snap, query)
    return plan(snap, query) ~= nil and net(snap, query) > 0
end

function M.value(snap, query)
    return net(snap, query) / plan(snap, query).span * combat.cast(snap) / 1000
end

function M.pending(snap, query)
    if M.wanted(snap, query) == false then
        return nil
    end
    return { gain = plan(snap, query).gain, trip = value.cost(snap, query, M.propose(snap, query),
        value.baseline(snap, query)) }
end

function M.propose(snap, query)
    local found      = plan(snap, query)
    local spot, move = near.approach(snap, query, found.npcs)
    if spot ~= nil then
        return { kind = ACTION.SAY, text = string.format("%s 1개 줘", found.name), gain = found.name }
    end
    return move or { kind = ACTION.HOLD }
end

return M
