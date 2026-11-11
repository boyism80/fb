local data   = require("brain.data.combat")
local combat = require("brain.policies.combat")
local danger = require("brain.policies.danger")
local escort = require("brain.policies.escort")
local ground = require("brain.policies.ground")
local score  = require("brain.policies.score")

local M = {}

-- The gap to a crewmate costs what being apart may lose over one action of mine: the hunting it does with the
-- crew, and its death rate alone on this floor times what its death costs. The death rate alone left no cost
-- at all on a floor where nobody dies alone, and a healer there never closed in. Inside the reach where
-- support ends the cost grows e-fold for every step the crewmate can walk while I act, so closing in matters
-- little beside it and more the farther it is: a crew kept within reach only by a hard limit let a poet heal
-- from behind a wall until its warrior walked off round it (offline profile trio rooted seed 1, 11-25s). Past
-- the reach it goes on at the slope it had there: grown e-fold per step without end, the walk round a wall to
-- a crewmate in sight outweighed healing that crewmate at 40% hp under blows (unit: a healer far from its
-- hurt crewmate). The crew hunts one plan, so my own worth prices every crewmate: a crewmate's copy has no
-- plan of its own and planning one for it on every decision made group lanes half again slower.
local scales = setmetatable({}, { __mode = "k" })

local function scale(snap, query, member)
    local known = scales[snap]
    if known == nil then
        known        = {}
        scales[snap] = known
    end
    local name = member.self.name or ""
    if known[name] == nil then
        local floor = ground.floor_at(member.world.map)
        local total = 0
        if floor ~= nil then
            local rested  = score.rested(member, ground.mode(member))
            local exposed = ground.evaluate(rested, query, floor, score.capability(rested, query), nil)
            local worth   = require("brain.policies.value").baseline(snap, query)
            total         = worth.value + (exposed.deaths or 0) * danger.loss(member, query, worth)
        end
        known[name] = total
    end
    return known[name]
end

local function crew(snap)
    local found = {}
    for _, member in ipairs(require("brain.policies.party").members(snap)) do
        if member.self.ghost == false and member.world.map == snap.world.map and member.world.x ~= nil then
            table.insert(found, member)
        end
    end
    return found
end

function M.weight(snap, steps)
    local stride = math.max(combat.cast(snap) / (snap.pace.step or data.walk), 1e-9)
    local over   = (steps - escort.reach()) / stride
    return over <= 0 and math.exp(over) or 1 + over
end
-- With several crewmates I stand where the sum is least, which is between them and nearer the one that
-- loses most alone; my own loss counts toward the nearest one, who would come to me first. The first tile
-- is where I stand: a crewmate I can reach from there and not from another tile makes that tile endless,
-- and one I cannot reach from there counts nowhere. Counted as no cost, a wall tile no crewmate reaches
-- looked like the best spot of all.
function M.costs(snap, query, tiles)
    local members = crew(snap)
    if #members == 0 or ground.floor_at(snap.world.map) == nil then
        return nil
    end
    local horizon = combat.cast(snap) / 1000
    local own     = scale(snap, query, snap)
    local fields  = {}
    local sizes   = {}
    for index, member in ipairs(members) do
        fields[index] = escort.steps(snap, query, member, tiles)
        sizes[index]  = scale(snap, query, member)
    end
    local result = {}
    for tile = 1, #tiles do
        local total   = 0
        local nearest = nil
        for index = 1, #members do
            local steps = fields[index][tile]
            if steps ~= false then
                local weight = M.weight(snap, steps)
                total        = total + sizes[index] * weight
                nearest      = math.min(nearest or weight, weight)
            elseif fields[index][1] ~= false then
                total = math.huge
            end
        end
        result[tile] = total == math.huge and math.huge or (total + own * (nearest or 0)) * horizon
    end
    return result
end

local places = setmetatable({}, { __mode = "k" })

-- The best tile within the steps I can walk now, off tiles taken by others and off tiles beside a mob that
-- strikes; its gain is the cost it saves against standing still.
function M.place(snap, query, budget)
    places[snap] = places[snap] or {}
    if places[snap][budget] ~= nil then
        return places[snap][budget] or nil
    end
    local found = nil
    if budget > 0 and snap.world.x ~= nil then
        local taken = {}
        for _, mob in ipairs(snap.world.mobs or {}) do
            if mob.x ~= nil then
                taken[mob.x .. "," .. mob.y] = true
                if combat.hits(mob) then
                    for _, offset in ipairs({ { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } }) do
                        taken[(mob.x + offset[1]) .. "," .. (mob.y + offset[2])] = true
                    end
                end
            end
        end
        for _, member in ipairs(crew(snap)) do
            taken[member.world.x .. "," .. member.world.y] = true
        end
        -- A tile that warps leaves the floor: a poet closing in on its rogue by the stairs of 흉가 93 stepped on
        -- the warp to 92, and the rally sent it back, five times in 10s (online team, 19:53).
        local tiles = { { x = snap.world.x, y = snap.world.y } }
        for dx = -budget, budget do
            for dy = -(budget - math.abs(dx)), budget - math.abs(dx) do
                local x, y = snap.world.x + dx, snap.world.y + dy
                if (dx ~= 0 or dy ~= 0) and x >= 0 and y >= 0 and taken[x .. "," .. y] == nil and
                    (query.floor == nil or query.floor(snap.world.map, x, y)) and
                    (query.warp == nil or query.warp(snap.world.map, x, y) == false) then
                    table.insert(tiles, { x = x, y = y })
                end
            end
        end
        local costs = M.costs(snap, query, tiles)
        if costs ~= nil then
            local walks = escort.steps(snap, query, snap, tiles)
            for index = 2, #tiles do
                local steps = walks[index]
                local gain  = costs[1] - costs[index]
                if steps ~= false and steps <= budget and gain > 0 and (found == nil or gain > found.gain) then
                    found = { x = tiles[index].x, y = tiles[index].y, steps = steps, gain = gain }
                end
            end
        end
    end
    places[snap][budget] = found or false
    return found
end

return M
