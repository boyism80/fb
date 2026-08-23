-- Seollal 보름달 (map 10946 '달맞이고개........').
-- Spawns at :05 on hours 0/4/8/12/16/20, walks east, then rewards and despawns.
local festival = require('lib.festival')

local M = {}

local MAP_ID = 10946
local TIMER_NAME = 'fullmoon'
local TIMER_PATH = 'scripts/lib/fullmoon.lua'
local INTERVAL_MS = 2800
local NPC_NAME = '보름달'
local START_X = 2
local START_Y = 2
local END_X = 195
local PROP_DONE = 'sesi.fullmoon_done'
local PROP_ACTIVE = 'sesi.fullmoon_active'
local PINWHEELS = { '파랑개비', '분홍개비', '하늘개비', '초록개비', '연두개비' }

local function slot_key(t)
    return string.format('%04d%02d%02d_%02d', t.year or 0, t.month or 0, t.day or 0, t.hour or 0)
end

local function is_spawn_window(t)
    local hour = t.hour or 0
    local minute = t.minute or 0
    return (hour % 4) == 0 and minute >= 5
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

local function destroy_moon(moon)
    if moon ~= nil then
        moon:destroy()
    end
end

-- Timer callbacks receive no map argument; resolve by id.
function M.on_tick()
    local map = id2map(MAP_ID)
    if map == nil then
        return
    end

    local moon = find_moon(map)

    if not festival.is('설날') then
        destroy_moon(moon)
        property(PROP_ACTIVE, nil)
        return
    end

    local t = datetime()
    if t == nil then
        return
    end

    if moon ~= nil then
        local x = moon:position()
        if x < END_X then
            moon:position(x + 1, START_Y)
            return
        end

        reward_seolbim_wearers(map)
        local completed = property(PROP_ACTIVE) or slot_key(t)
        destroy_moon(moon)
        property(PROP_DONE, completed)
        property(PROP_ACTIVE, nil)
        return
    end

    property(PROP_ACTIVE, nil)

    if not is_spawn_window(t) then
        return
    end

    local slot = slot_key(t)
    if property(PROP_DONE) == slot then
        return
    end

    property(PROP_ACTIVE, slot)
    mknpc(NPC_NAME, map, START_X, START_Y)
end

function M.start(map)
    if map == nil then
        map = id2map(MAP_ID)
    end
    if map == nil then
        return
    end

    -- Drop leftover static/spawned moons from previous logic.
    destroy_moon(find_moon(map))
    property(PROP_ACTIVE, nil)

    map:cancel_timer(TIMER_NAME)
    map:set_timer(INTERVAL_MS, TIMER_PATH, 'on_tick', { name = TIMER_NAME })
end

return M
