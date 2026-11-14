local memo = require("brain.policies.memo")

local M = {}

local grids   = {}
local detours = {}

function M.floor(source, map, x, y)
    local grid = grids[map]
    if grid == nil then
        grid       = source:grid(map) or false
        grids[map] = grid
    end
    if grid == false or x < 0 or y < 0 or x >= grid.width or y >= grid.height then
        return false
    end
    return string.byte(grid.blocked, y * grid.width + x + 1) == 0
end

-- What I see standing about counts for a walk I start from where I stand, not for a crewmate's walk or the gap
-- between us: those bodies move, and a wall stays. Counted for the gap, a poet whose warrior mobs had closed round
-- followed the walk round the floor away from it, one step a cast, and the trio stood apart for 296s (offline
-- profile trio seed 0, 602-900s, run 14:12).
function M.blocking(snap, map, x, y)
    local world = snap ~= nil and snap.world or nil
    if world == nil or world.map ~= map or world.x ~= x or world.y ~= y then
        return nil
    end
    return world.occupied
end

-- The walker steps round every mob and character it sees and only walks into one when no way round is left, so
-- a walk weighed on bare ground is not the walk taken: 4 steps to the mob on the poet became 35 tiles and 13s
-- away once the tiles round the mob filled (WA63+PO65 seed 2, 824-838s, run 23:41). A goal tile someone stands
-- on still counts as reached, as the walker never avoids its target; when no way round is left the walker
-- walks the bare path up to whoever stands in it, so that count stands. Crewmates are not in the way: they make
-- way (together.make_way). Counted as walls, a poet in the one-tile way to the stairs of 흉가 91 turned its
-- rogue's 5 steps into the walk round the floor and back with every step either took, and the trio never left
-- (unit: trio moves to the next floor without bouncing).
-- A strike walk is the hunter's: it walks round whoever stands in the way within the steps it was weighed with and
-- ends UNREACHABLE when no way round is left, so a strike is weighed strict, with no bare walk to fall back on.
-- Weighed by the bare walk, a poet freeing its rogue from the mobs round it struck 673 times at mobs it could not
-- reach and 389 of them ended UNREACHABLE at once (offline RO+PO 세작, run 09:06).
function M.steps(source, map, x, y, goals, occupied, traced, strict)
    if occupied == nil or #occupied == 0 then
        return source:steps(map, x, y, goals, nil, traced)
    end
    local ends = {}
    for _, goal in ipairs(goals) do
        if (goal.range or 0) == 0 then
            ends[goal.x .. "," .. goal.y] = true
        end
    end
    local avoided = {}
    for _, tile in ipairs(occupied) do
        if ends[tile.x .. "," .. tile.y] == nil then
            table.insert(avoided, tile)
        end
    end
    local found, routes = source:steps(map, x, y, goals, avoided, traced)
    local bare, ways    = nil, nil
    for index = 1, #goals do
        if found[index] == false and strict ~= true then
            if bare == nil then
                bare, ways = source:steps(map, x, y, goals, nil, traced)
            end
            found[index] = bare[index]
            if routes ~= nil then
                routes[index] = ways[index]
            end
        end
    end
    return found, routes
end

function M.detour(source, map, from, to, avoided)
    local spots = string.format("%d,%d|%d,%d", from.x, from.y, avoided.x, avoided.y)
    return memo.get(detours, tostring(map), to.x .. "," .. to.y, spots, function()
        local goal   = { { x = to.x, y = to.y, range = 0 } }
        local free   = source:steps(map, from.x, from.y, goal)[1]
        local around = source:steps(map, from.x, from.y, goal, { { x = avoided.x, y = avoided.y } })[1]
        if type(free) ~= "number" then
            return 0
        end
        return type(around) == "number" and around - free or math.huge
    end)
end

return M
