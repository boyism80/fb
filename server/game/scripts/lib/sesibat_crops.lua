-- 세시밭 ground-item crop spawn (콩/조/피). Active during 유두.
local festival = require('lib.festival')

local M = {}

local MAP_ID = 10955
local TIMER_NAME = 'sesibat_crops'
local TIMER_PATH = 'scripts/lib/sesibat_crops.lua'
local INTERVAL_MS = 300000

-- { name, zones = { {x1, x2, y1, y2, target}, ... } }
local CROPS = {
    {
        name = '콩',
        zones = {
            { 21, 23, 22, 28, 7 },
            { 11, 15, 20, 22, 5 },
            { 11, 15, 24, 26, 5 },
            { 1, 5, 20, 22, 5 },
            { 1, 5, 24, 26, 5 },
            { 1, 5, 16, 18, 5 },
            { 26, 27, 22, 28, 5 },
            { 11, 15, 16, 18, 5 },
        },
    },
    {
        name = '조',
        zones = {
            { 11, 17, 62, 64, 10 },
            { 11, 17, 70, 72, 9 },
            { 30, 36, 61, 63, 8 },
            { 11, 17, 66, 68, 8 },
            { 18, 25, 54, 55, 8 },
            { 18, 25, 57, 58, 7 },
            { 30, 36, 52, 53, 6 },
            { 30, 36, 58, 59, 6 },
            { 38, 41, 71, 73, 6 },
            { 38, 41, 67, 69, 5 },
            { 31, 35, 67, 69, 5 },
            { 31, 35, 71, 73, 5 },
            { 30, 33, 55, 56, 3 },
            { 11, 13, 47, 48, 3 },
            { 11, 13, 50, 51, 3 },
            { 34, 36, 55, 56, 3 },
            { 11, 13, 53, 54, 3 },
            { 35, 35, 73, 73, 1 },
        },
    },
    {
        name = '피',
        zones = {
            { 50, 50, 47, 56, 10 },
            { 52, 55, 30, 32, 10 },
            { 53, 53, 48, 56, 9 },
            { 49, 55, 58, 58, 7 },
            { 35, 38, 23, 24, 6 },
            { 49, 51, 23, 24, 5 },
            { 49, 51, 34, 35, 5 },
            { 49, 51, 20, 21, 5 },
            { 47, 47, 48, 52, 5 },
            { 49, 51, 26, 27, 4 },
            { 56, 56, 53, 56, 4 },
            { 47, 47, 54, 56, 3 },
            { 49, 49, 30, 32, 3 },
            { 50, 52, 60, 60, 3 },
        },
    },
}

local function in_zone(x, y, zone)
    return x >= zone[1] and x <= zone[2] and y >= zone[3] and y <= zone[4]
end

local function count_crop_in_zone(map, item_name, zone)
    local n = 0
    for _, item in pairs(map:objects(OBJECT_TYPE.ITEM)) do
        if item:name() == item_name then
            local x, y = item:position()
            if in_zone(x, y, zone) then
                n = n + 1
            end
        end
    end
    return n
end

local function try_spawn_one(map, item_name, zone)
    for _ = 1, 40 do
        local x = math.random(zone[1], zone[2])
        local y = math.random(zone[3], zone[4])
        if map:at(x, y, OBJECT_TYPE.ITEM) == nil and not map:block(x, y) then
            local item = map:mkitem(item_name, x, y)
            if item ~= nil then
                return true
            end
        end
    end
    return false
end

function M.start(map)
    if map == nil then
        map = id2map(MAP_ID)
    end
    if map == nil then
        return false
    end
    local id = map:set_timer(INTERVAL_MS, TIMER_PATH, 'on_tick', { name = TIMER_NAME })
    M.on_tick()
    return id ~= nil
end

function M.on_tick()
    if not festival.is('유두') then
        return
    end

    local map = id2map(MAP_ID)
    if map == nil then
        return
    end

    for _, crop in ipairs(CROPS) do
        for _, zone in ipairs(crop.zones) do
            local alive = count_crop_in_zone(map, crop.name, zone)
            local need = zone[5] - alive
            for _ = 1, need do
                if not try_spawn_one(map, crop.name, zone) then
                    break
                end
            end
        end
    end
end

return M
