-- Seollal 보름달무브 (map 10946).
local festival = require('lib.festival')

local M = {}

local MAP_ID = 10946
local TIMER_NAME = 'fullmoon'
local TIMER_PATH = 'scripts/lib/fullmoon.lua'
local INTERVAL_MS = 2800
local NPC_NAME = '보름달'
local PINWHEELS = { '파랑개비', '분홍개비', '하늘개비', '초록개비', '연두개비' }

local function today_key()
    local t = datetime()
    if t == nil then
        return nil
    end
    return (t.year or 0) * 10000 + (t.month or 0) * 100 + (t.day or 0)
end

local function find_moon(map)
    for _, npc in pairs(map:objects(OBJECT_TYPE.NPC)) do
        if npc:name() == NPC_NAME then
            return npc
        end
    end
    return nil
end

local function wearing_seolbim(ch)
    local eqs = ch:equipments()
    if eqs == nil then
        return false
    end
    local armor = eqs[EQUIPMENT_PARTS.ARMOR]
    if armor == nil then
        return false
    end
    local name = armor:name()
    return name == '남자설빔' or name == '여자설빔'
end

local function reward_seolbim_wearers(map)
    for _, ch in pairs(map:objects(OBJECT_TYPE.CHARACTER)) do
        if wearing_seolbim(ch) then
            local item = PINWHEELS[math.random(1, #PINWHEELS)]
            if ch:mkitem(item, 1) ~= nil then
                ch:effect(131)
                ch:sound(112)
                ch:message(item .. ' 받았습니다.', MESSAGE_TYPE.NOTIFY)
            end
        end
    end
end

function M.on_tick(map)
    if map == nil then
        return
    end

    local moon = find_moon(map)
    if moon == nil then
        return
    end

    if not festival.is('설날') then
        moon:position(199, 39)
        return
    end

    local t = datetime()
    if t == nil then
        return
    end

    local today = today_key()
    local last = property('sesi.fullmoon_day')
    local hour = t.hour or 0
    local moving = today ~= nil and (last == nil or today > last) and hour == 0
    if not moving then
        moon:position(199, 39)
        return
    end

    local x, y = moon:position()
    if x < 195 then
        moon:position(x + 1, y)
        return
    end

    if x == 199 and y == 39 then
        moon:position(2, 2)
    elseif last ~= today then
        reward_seolbim_wearers(map)
        property('sesi.fullmoon_day', today)
    else
        moon:position(199, 39)
    end
end

function M.start(map)
    if map == nil then
        map = game_map(MAP_ID)
    end
    if map == nil then
        return
    end
    map:cancel_timer(TIMER_NAME)
    map:set_timer(INTERVAL_MS, TIMER_PATH, 'on_tick', { name = TIMER_NAME })
end

return M
