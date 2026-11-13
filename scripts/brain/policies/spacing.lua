local data   = require("brain.data.combat")
local combat = require("brain.policies.combat")
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
            total         = worth.value +
            (exposed.deaths or 0) * require("brain.policies.party").death(snap, query, member, worth)
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
    local far    = {}
    for tile = 1, #tiles do
        local total   = 0
        local nearest = nil
        local walk    = 0
        for index = 1, #members do
            local steps = fields[index][tile]
            if steps ~= false then
                local weight = M.weight(snap, steps)
                total        = total + sizes[index] * weight
                nearest      = math.min(nearest or weight, weight)
                walk         = math.max(walk, steps)
            elseif fields[index][1] ~= false then
                total = math.huge
            end
        end
        result[tile] = total == math.huge and math.huge or (total + own * (nearest or 0)) * horizon
        far[tile]    = total == math.huge and math.huge or walk
    end
    return result, far
end

local OFFSETS = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } }

-- A tile costs the blows I take there over one action of mine, priced by what my hp costs to win back: a mob that
-- comes for me strikes once it has walked to my side, and one beside the tile that comes for a crewmate it cannot
-- step toward strikes whoever stands beside it, as the server's mobs do (aggressive_ai). The walk there costs the
-- blows of the steps beside a mob that comes for me. With the tiles beside a striking mob only left out, a poet
-- the skeleton beside it had come for healed and swung where it stood (user 11-12: poets and mages keep off the
-- mobs and keep the crew's order).
local function blows(snap, query, tiles, routes, taken)
    local party    = require("brain.policies.party")
    local value    = require("brain.policies.value")
    local horizon  = combat.cast(snap)
    local step     = snap.pace.step or data.walk
    local strikers = {}
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and combat.hits(mob) and (mob.speed or 0) > 0 then
            table.insert(strikers,
                { mob = mob, rate = combat.blow(snap, mob) / mob.speed, now = party.chaser(snap, mob, snap.world) })
        end
    end
    local result = {}
    if #strikers == 0 then
        for index = 1, #tiles do
            result[index] = 0
        end
        return result
    end
    local worth = value.baseline(snap, query)
    local price = value.vigor(snap, query, worth, value.spend(snap, query, nil))
    local function stuck(mob, target, tile)
        local gap = math.abs(mob.x - target.x) + math.abs(mob.y - target.y)
        if gap <= 1 then
            return false
        end
        for _, offset in ipairs(OFFSETS) do
            local x, y = mob.x + offset[1], mob.y + offset[2]
            if math.abs(x - target.x) + math.abs(y - target.y) < gap and taken[x .. "," .. y] == nil and
                (x ~= tile.x or y ~= tile.y) and (query.floor == nil or query.floor(snap.world.map, x, y)) then
                return false
            end
        end
        return true
    end
    for index, tile in ipairs(tiles) do
        local damage = 0
        for _, striker in ipairs(strikers) do
            local mob    = striker.mob
            local chaser = party.chaser(snap, mob, tile)
            local gap    = math.abs(mob.x - tile.x) + math.abs(mob.y - tile.y)
            if chaser ~= nil and chaser.name == snap.self.name then
                damage = damage + striker.rate * math.max(0, horizon - math.max(0, gap - 1) * mob.speed)
            elseif chaser ~= nil and gap == 1 and stuck(mob, chaser.world, tile) then
                damage = damage + striker.rate * horizon
            end
            if striker.now ~= nil and striker.now.name == snap.self.name then
                for _, at in ipairs(routes ~= nil and routes[index] or {}) do
                    if math.abs(mob.x - at.x) + math.abs(mob.y - at.y) == 1 then
                        damage = damage + striker.rate * step
                    end
                end
            end
        end
        result[index] = damage * price
    end
    return result
end

local function beside(snap)
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
            return true
        end
    end
    return false
end

local function nothing(count)
    local result = {}
    for index = 1, count do
        result[index] = 0
    end
    return result
end

-- A crewmate on another floor is past every tile of mine, and the walk to it is the rally's. With the blows alone
-- left to weigh, a poet two tiles from the stairs its rogue had crossed stepped back off the skeleton on it after
-- every blow and took that step again with the rally, 15s on the spot (offline profile 50+65 form seed 1, 629-644s,
-- run 20:51).
local function parted(snap)
    for _, member in ipairs(require("brain.policies.party").members(snap)) do
        local map = member.world.map
        if member.self.ghost == false and map ~= snap.world.map and ground.floor_at(map) ~= nil then
            return true
        end
    end
    return false
end

local function walled(snap, query, tile)
    if query.detour == nil or query.floor == nil then
        return false
    end
    for _, offset in ipairs(OFFSETS) do
        if query.floor(snap.world.map, tile.x + offset[1], tile.y + offset[2]) == false then
            return true
        end
    end
    return false
end

local places = setmetatable({}, { __mode = "k" })

-- The best tile within the steps I can walk now, off tiles taken by others; its gain is the cost it saves
-- against standing still, the crew's gaps and the blows there together. While every crewmate stays within the reach
-- the crew's order holds (escort.reach), the tile with the fewest blows comes first and the nearer one next. Weighed
-- as one sum there, a tile farther from a crewmate cost 70-150 more and a mob's blow 15-190, as its own heal makes a
-- poet's hp cheap: with a free tile beside it a poet stayed under the mob on it 80 times in 88 (offline 25 teams,
-- run 19:16; user 11-12: poets and mages keep off the mobs within the crew's order). Past that reach the sum
-- decides, as getting back comes before the blows. With no crew on the floor there is no gap
-- to keep and the blows alone weigh the tile (user 11-12: a bot alone keeps off the mobs as well). Left out of the
-- choice, a bot alone cast where the mob beside it struck: mages alone took 115,006 hp for 1,082 kills over six
-- 15-minute seeds (offline MA and profile mage60, run 20:18).
function M.place(snap, query, budget)
    places[snap] = places[snap] or {}
    if places[snap][budget] ~= nil then
        return places[snap][budget] or nil
    end
    local found = nil
    if budget > 0 and snap.world.x ~= nil and parted(snap) == false then
        local taken = {}
        for _, tile in ipairs(snap.world.occupied or {}) do
            taken[tile.x .. "," .. tile.y] = true
        end
        for _, mob in ipairs(snap.world.mobs or {}) do
            if mob.x ~= nil then
                taken[mob.x .. "," .. mob.y] = true
            end
        end
        -- The tile a crewmate's walk stopped at is on its way and no place of mine: the tile south of their warrior,
        -- its only way to the stairs of 흉가 92, was the best place beside it for its rogue and its poet, and one of
        -- them was back on it at each of the warrior's tries for 400s (offline profile trio rooted seed 6, 320-740s,
        -- run 22:38, walker held to the judged way).
        local members = crew(snap)
        local mates   = {}
        for _, member in ipairs(members) do
            taken[member.world.x .. "," .. member.world.y] = true
            mates[member.world.x .. "," .. member.world.y] = true
            if member.blocker ~= nil then
                taken[member.blocker.x .. "," .. member.blocker.y] = true
            end
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
        -- Beside a mob I stay when my strongest blow is struck from its side. Weighed with the crew's gaps, a warrior
        -- whose poet stood six tiles off stepped off the skeleton it fought, as a step nearer was worth 80 times its
        -- swing (unit: a mob beside me keeps a striker); held beside every mob, a poet healed where the skeleton beside
        -- it struck it (user 11-12: poets and mages keep off the mobs). Such a bot takes no tile for the blows either:
        -- the mob it steps from follows, so the blow is put off, not spared. Weighed for them, warriors and rogues
        -- alone stepped back from the mobs their 백열장 had called and killed 38,428 for 40,736 (offline 120 solos,
        -- runs 21:06 and 19:16).
        local melee      = score.melee(snap, query)
        local costs, far = nil, nil
        if melee == false or beside(snap) == false then
            costs, far = M.costs(snap, query, tiles)
        end
        if costs ~= nil or melee == false then
            costs       = costs or nothing(#tiles)
            local goals = {}
            for _, tile in ipairs(tiles) do
                table.insert(goals, { x = tile.x, y = tile.y, range = 0 })
            end
            local walks, routes = nil, nil
            if query.steps ~= nil then
                walks, routes = query.steps(snap.world.map, snap.world.x, snap.world.y, goals, true)
            else
                walks = escort.steps(snap, query, snap, tiles)
            end
            local struck = melee and nothing(#tiles) or blows(snap, query, tiles, routes, taken)
            local reach  = escort.reach()
            local kept   = far == nil or far[1] <= reach
            for index = 2, #tiles do
                local steps = type(walks[index]) == "number" and walks[index] or false
                -- A step off is walked in the gap between casts, so a tile reached only through a crewmate's tile is
                -- no step: the walk stopped at the warrior, which made way onto that very tile, and the poet ended
                -- between its warrior and the skeletons, struck for 92s (offline profile 63+65 form seed 0, 735-828s,
                -- run 20:51).
                for _, at in ipairs(routes ~= nil and routes[index] or {}) do
                    steps = steps ~= false and mates[at.x .. "," .. at.y] == nil and steps
                end
                -- A tile on a crewmate's only way to where I stand shuts it in: a rogue walked into a dead end of 흉가 91
                -- to cast across its wall, its poet stepped after it to the mouth, and for 300s the mobs that came
                -- for the rogue struck the poet while the rogue reached none (offline RO+PO seed 1, 410-720s, run
                -- 01:54). A way round, however long, leaves the crewmate its way out.
                for _, member in ipairs(members) do
                    steps = steps ~= false and
                        (walled(snap, query, tiles[index]) == false or
                            query.detour(snap.world.map, member.world, snap.world, tiles[index]) ~= math.huge) and steps
                end
                local spared = struck[1] - struck[index]
                local closer = costs[1] - costs[index]
                local gain   = costs[1] + struck[1] - costs[index] - struck[index]
                local ahead  = false
                if steps ~= false and steps <= budget and kept then
                    ahead = (far == nil or far[index] <= reach) and (spared > 0 or (spared == 0 and closer > 0)) and
                        (found == nil or spared > found.spared or (spared == found.spared and closer > found.closer))
                    -- Within the reach the crew's order holds wherever I stand, so the step is worth what it spares and
                    -- any gap it closes, never charged for the gap it opens: summed with that gap, a tile one step
                    -- farther from the warrior cost 1,360-3,082 against 135-747 of blows spared, and the poet swung
                    -- where it stood in 40 of the 46 moments a better tile was there (online team, run 03:11, rebuilt).
                    gain  = spared + math.max(closer, 0)
                elseif steps ~= false and steps <= budget then
                    ahead = gain > 0 and (found == nil or gain > found.gain)
                end
                if ahead then
                    found = {
                        x = tiles[index].x,
                        y = tiles[index].y,
                        steps = steps,
                        gain = gain,
                        spared = spared,
                        closer = closer
                    }
                end
            end
        end
    end
    places[snap][budget] = found or false
    return found
end

return M
