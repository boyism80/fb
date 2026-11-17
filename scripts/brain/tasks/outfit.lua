local outfits = require("brain.data.outfits")
local combat  = require("brain.policies.combat")
local failure = require("brain.policies.failure")
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
        phydef = (snap.self.phydef or 0) - stat(query, name, "defense") + stat(query, was, "defense"),
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
            -- The path uses this bot's level for warp limits, so the level is part of the key. Both ways are
            -- asked: 환상의섬대장장이 can be walked away from but not to, and its weapon, picked as the best one
            -- because the way back exists, left a mainland rogue buying nothing for the whole run.
            local key = table.concat({ name, spot.map, table.concat(goal.maps, ","), snap.self.level or 0 }, "|")
            if links[key] == nil then
                links[key] = query.path({ maps = goal.maps }, nil, nil, spot) ~= nil and
                    query.path({ maps = { spot.map } }, nil, nil,
                        { map = goal.maps[1], x = 0, y = 0, level = snap.self.level }) ~= nil
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
        local now     = ground.choose(snap, query)
        local held    = now ~= nil and ground.hold({ snap }, query, now.name) or nil
        local rate    = held ~= nil and held.rate or 0
        -- The gain is measured against the current ground, so the ground name is part of the key.
        local base    = score.signature(snap) .. "|" .. tostring(now ~= nil and now.name or nil)
        local choices = snap.record.death == nil and picks(snap, query) or {}
        local kinds   = {}
        for kind in pairs(choices) do
            table.insert(kinds, kind)
        end
        table.sort(kinds)
        for _, kind in ipairs(kinds) do
            local pick = choices[kind]
            -- Always measured: skipping it when a per-decision budget ran out made the same state
            -- buy gear on one tick and not on the next.
            local gain = memo.get(gains, snap.self.name, base, pick.name, function()
                local after = now ~= nil and ground.hold({ dressed(snap, query, kind, pick.name) }, query, now.name) or
                    nil
                return math.max(0, (after ~= nil and after.rate or 0) - rate)
            end)
            -- A lasting gain pays any price in the end, so what is asked is whether the money is there and when to go
            -- (worth): the biggest gain first. Weighed as the gain until the next level less the price, it was worth
            -- nothing to read at 99 (ground.lap).
            if (gain or 0) > 0 and pick.outfit.price <= (snap.self.money or 0) then
                if best == nil or gain > best.gain or (gain == best.gain and pick.name < best.name) then
                    best = { name = pick.name, npcs = pick.npcs, gain = gain }
                end
            end
        end
    end
    plans[snap] = best or false
    return best
end

-- Going now against buying it on the next servicing: the trip, with the crew waiting for it, is spread over what is
-- left of the lap, since put off it costs the gain until then and nothing more (at least the trip itself, so a
-- servicing due now still pays for the walk).
local function worth(snap, query)
    local found  = plan(snap, query)
    local action = M.next_step(snap, query)
    local rates  = value.baseline(snap, query)
    local cost   = value.cost(snap, query, action, rates) * (1 + #party.members(snap))
    local trip   = action.kind == ACTION.MOVE and action.dest ~= nil and value.trip(snap, query, action.dest, rates) or
        nil
    local window = math.max(ground.lap_left(snap, party.plan(snap, query)), trip ~= nil and trip.time or 0,
        combat.cast(snap) / 1000)
    return found.gain * value.arrival(snap, query, action, rates) - (cost > 0 and cost / window or 0)
end

function M.has_plan(snap, query)
    return plan(snap, query) ~= nil
end

function M.pays(snap, query)
    return worth(snap, query) > 0
end

function M.value(snap, query)
    return worth(snap, query) * combat.cast(snap) / 1000
end

-- Whether the errand would be taken now, the one rule the tree and a held invite both ask: told apart, the invite
-- waited on gear whose worth read NaN and was never walked for (offline 20261115-104514).
function M.ready(snap, query)
    return M.has_plan(snap, query) and M.pays(snap, query) and failure.allowed(snap, "OUTFIT")
end

function M.pending(snap, query)
    if M.ready(snap, query) == false then
        return nil
    end
    return {
        gain = plan(snap, query).gain,
        trip = value.cost(snap, query, M.next_step(snap, query), value.baseline(snap, query))
    }
end

function M.next_step(snap, query)
    local found      = plan(snap, query)
    local spot, move = near.approach(snap, query, found.npcs)
    if spot ~= nil then
        return { kind = ACTION.SAY, text = string.format("%s 1개 줘", found.name), gain = found.name }
    end
    return move or { kind = ACTION.HOLD }
end

return M
