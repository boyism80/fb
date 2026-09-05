-- Timed pier boat (NPC 배). Arrive LEAD_SEC before boarding, sit at dock
-- during the window, then leave along the sea axis. Boarding is odd hours
-- 40:00-59:59 (same as the sailor scripts).
local M = {}

local NPC_ID = 80
local NPC_NAME = '배'
local TIMER_NAME = 'ferry_boat'
local TIMER_PATH = 'scripts/lib/ferry_boat.lua'
local INTERVAL_MS = 1000

-- Independent knobs: seconds before/after the boarding window vs tiles travelled.
local TRAVEL_TILES = 10
local ARRIVE_LEAD_SEC = 10

local AXIS_Y = 'y'
local AXIS_X = 'x'

-- Sea is the depart direction from the dock. Facing matches movement:
-- arrive/dock = arrive_dir, depart = depart_dir.
local ROUTES = {
    [1744] = { -- Buyeo Gogyundo pier
        dock_x = 21, dock_y = 38, axis = AXIS_Y,
        arrive_dir = DIRECTION.TOP, depart_dir = DIRECTION.BOTTOM,
    },
    [1746] = { -- Goguryeo Gogyundo pier
        dock_x = 21, dock_y = 38, axis = AXIS_Y,
        arrive_dir = DIRECTION.TOP, depart_dir = DIRECTION.BOTTOM,
    },
    [1822] = { -- Buyeo Japan pier
        dock_x = 60, dock_y = 21, axis = AXIS_X,
        arrive_dir = DIRECTION.LEFT, depart_dir = DIRECTION.RIGHT,
    },
    [1809] = { -- Goguryeo Japan pier
        dock_x = 60, dock_y = 21, axis = AXIS_X,
        arrive_dir = DIRECTION.LEFT, depart_dir = DIRECTION.RIGHT,
    },
}

local function clamp(v, lo, hi)
    if v < lo then
        return lo
    end
    if v > hi then
        return hi
    end
    return v
end

local function sea_point(route, map)
    local x = route.dock_x
    local y = route.dock_y
    if route.axis == AXIS_Y then
        y = y + TRAVEL_TILES
    else
        x = x + TRAVEL_TILES
    end
    local width = map:width()
    local height = map:height()
    if width ~= nil and width > 0 then
        x = clamp(x, 0, width - 1)
    end
    if height ~= nil and height > 0 then
        y = clamp(y, 0, height - 1)
    end
    return x, y
end

local function lerp_to_sea(route, map, along)
    local sea_x, sea_y = sea_point(route, map)
    local dx = sea_x - route.dock_x
    local dy = sea_y - route.dock_y
    local span = TRAVEL_TILES
    if span <= 0 then
        return route.dock_x, route.dock_y
    end
    along = clamp(along, 0, span)
    local x = route.dock_x + math.floor(dx * along / span + 0.5)
    local y = route.dock_y + math.floor(dy * along / span + 0.5)
    return x, y
end

-- along 0 = dock, along TRAVEL_TILES = sea.
-- arriving: along goes TRAVEL_TILES -> 0. departing: 0 -> TRAVEL_TILES.
local function phase_at(t)
    local hour = t.hour or 0
    local minute = t.minute or 0
    local second = t.second or 0
    local lead = ARRIVE_LEAD_SEC
    if lead < 1 then
        lead = 1
    end

    if (hour % 2) == 1 then
        if minute >= 40 then
            return 'docked', 0
        end
        local until_board = 40 * 60 - (minute * 60 + second)
        if until_board > 0 and until_board <= lead then
            local elapsed = lead - until_board
            local along = TRAVEL_TILES - math.floor(elapsed * TRAVEL_TILES / lead + 0.5)
            return 'arriving', clamp(along, 0, TRAVEL_TILES)
        end
        return 'absent', TRAVEL_TILES
    end

    if minute == 0 and second <= lead then
        local along = math.floor(second * TRAVEL_TILES / lead + 0.5)
        return 'departing', clamp(along, 0, TRAVEL_TILES)
    end
    return 'absent', TRAVEL_TILES
end

local function face_for(route, phase)
    if phase == 'departing' then
        return route.depart_dir
    end
    return route.arrive_dir
end

local function collect_boats(map)
    local boats = {}
    for _, npc in pairs(map:objects(OBJECT_TYPE.NPC)) do
        if npc:model():id() == NPC_ID then
            boats[#boats + 1] = npc
        end
    end
    return boats
end

local function destroy_boats(boats)
    for _, boat in ipairs(boats) do
        boat:destroy()
    end
end

local function step_toward(boat, tx, ty)
    local x, y = boat:position()
    if x == tx and y == ty then
        return true
    end

    local dir
    if y ~= ty then
        if ty < y then
            dir = DIRECTION.TOP
        else
            dir = DIRECTION.BOTTOM
        end
    else
        if tx < x then
            dir = DIRECTION.LEFT
        else
            dir = DIRECTION.RIGHT
        end
    end

    if boat:move(dir) == true then
        return true
    end

    local nx, ny = x, y
    if dir == DIRECTION.TOP then
        ny = y - 1
    elseif dir == DIRECTION.BOTTOM then
        ny = y + 1
    elseif dir == DIRECTION.LEFT then
        nx = x - 1
    else
        nx = x + 1
    end
    boat:position(nx, ny)
    boat:direction(dir)
    return true
end

function M.sync(map_id)
    local route = ROUTES[map_id]
    if route == nil then
        return
    end

    local map = id2map(map_id)
    if map == nil then
        return
    end

    local t = datetime()
    if t == nil then
        return
    end

    local phase, along = phase_at(t)
    local boats = collect_boats(map)
    if phase == 'absent' then
        destroy_boats(boats)
        return
    end

    local tx, ty = lerp_to_sea(route, map, along)
    local dir = face_for(route, phase)

    local boat = boats[1]
    for i = 2, #boats do
        boats[i]:destroy()
    end

    if boat == nil then
        mknpc(NPC_NAME, map, tx, ty, dir)
        return
    end

    local x, y = boat:position()
    if x == tx and y == ty then
        if boat:direction() ~= dir then
            boat:direction(dir)
        end
        return
    end

    local dist = math.abs(x - tx) + math.abs(y - ty)
    if dist > 2 then
        boat:position(tx, ty)
        boat:direction(dir)
        return
    end

    local guard = 0
    while guard < TRAVEL_TILES do
        guard = guard + 1
        x, y = boat:position()
        if x == tx and y == ty then
            break
        end
        dist = math.abs(x - tx) + math.abs(y - ty)
        if dist > 2 then
            boat:position(tx, ty)
            boat:direction(dir)
            break
        end
        local before_x, before_y = x, y
        step_toward(boat, tx, ty)
        x, y = boat:position()
        if x == before_x and y == before_y then
            boat:position(tx, ty)
            break
        end
    end
    if boat:direction() ~= dir then
        boat:direction(dir)
    end
end

function M.start(map)
    if map == nil then
        return
    end
    local id = map:model():id()
    if ROUTES[id] == nil then
        return
    end

    map:cancel_timer(TIMER_NAME)
    map:set_timer(INTERVAL_MS, TIMER_PATH, 'on_tick_' .. id, { name = TIMER_NAME })
    M.sync(id)
end

function M.on_tick_1744()
    M.sync(1744)
end

function M.on_tick_1746()
    M.sync(1746)
end

function M.on_tick_1822()
    M.sync(1822)
end

function M.on_tick_1809()
    M.sync(1809)
end

return M
