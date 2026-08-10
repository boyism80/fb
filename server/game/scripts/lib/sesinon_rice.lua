-- 세시논 ground-item rice spawn. Active during 칠석 and 유두.
local festival = require('lib.festival')

local M = {}

local MAP_ID = 25227
local TIMER_NAME = 'sesinon_rice'
local TIMER_PATH = 'scripts/lib/sesinon_rice.lua'
local INTERVAL_MS = 300000
local ITEM_NAME = '벼'

-- {x1, x2, y1, y2, target}
local ZONES = {
    { 9, 23, 1, 10, 10 },
    { 28, 38, 1, 6, 5 },
    { 43, 53, 1, 4, 5 },
    { 13, 23, 14, 19, 5 },
    { 28, 38, 11, 17, 5 },
    { 13, 23, 23, 28, 5 },
    { 11, 37, 48, 58, 20 },
    { 11, 37, 63, 70, 15 },
    { 52, 61, 20, 25, 5 },
    { 52, 61, 30, 35, 5 },
    { 52, 61, 40, 45, 5 },
    { 52, 61, 50, 55, 5 },
    { 52, 61, 60, 65, 5 },
    { 52, 61, 70, 75, 5 },
    { 66, 74, 20, 25, 5 },
    { 66, 74, 30, 35, 5 },
    { 66, 74, 40, 45, 5 },
    { 66, 74, 50, 55, 5 },
    { 66, 74, 60, 65, 5 },
    { 66, 74, 70, 75, 5 },
}

local function in_zone(x, y, zone)
    return x >= zone[1] and x <= zone[2] and y >= zone[3] and y <= zone[4]
end

local function count_rice_in_zone(map, zone)
    local n = 0
    for _, item in pairs(map:objects(OBJECT_TYPE.ITEM)) do
        if item:name() == ITEM_NAME then
            local x, y = item:position()
            if in_zone(x, y, zone) then
                n = n + 1
            end
        end
    end
    return n
end

local function try_spawn_one(map, zone)
    for _ = 1, 40 do
        local x = math.random(zone[1], zone[2])
        local y = math.random(zone[3], zone[4])
        if map:at(x, y, OBJECT_TYPE.ITEM) == nil and not map:block(x, y) then
            local item = map:mkitem(ITEM_NAME, x, y)
            if item ~= nil then
                return true
            end
        end
    end
    return false
end

function M.start(map)
    if map == nil then
        map = game_map(MAP_ID)
    end
    if map == nil then
        return false
    end
    local id = map:set_timer(INTERVAL_MS, TIMER_PATH, 'on_tick', { name = TIMER_NAME })
    -- Fill missing rice immediately so the map is not empty for up to 300s.
    M.on_tick()
    return id ~= nil
end

function M.on_tick()
    if not festival.is('칠석') and not festival.is('유두') then
        return
    end

    local map = game_map(MAP_ID)
    if map == nil then
        return
    end

    for _, zone in ipairs(ZONES) do
        local alive = count_rice_in_zone(map, zone)
        local need = zone[5] - alive
        for _ = 1, need do
            if not try_spawn_one(map, zone) then
                break
            end
        end
    end
end

return M
