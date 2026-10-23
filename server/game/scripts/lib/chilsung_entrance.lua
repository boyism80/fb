-- Chilseok/Seollal/Yudu 칠성당 entrance gate.
local festival = require('lib.festival')

local M = {}

local OFFERINGS_CHILSEOK = { '벼', '호박부침', '잉어구이' }
local OFFERINGS_YUDU = { '유두벼', '유두콩', '유두조' }

local function push_back(me)
    local map = me:map()
    if map == nil then
        return
    end
    local x, y = me:position()
    me:message('들어갈 수 없습니다...', MESSAGE_TYPE.STATE)
    me:map(map, x, y + 1)
end

local function enterable()
    local t = datetime()
    if t == nil then
        return false
    end
    return festival.is_triple_hour(t.hour or 0) and (t.minute or 0) <= 30
end

local function enter_chilsung(me)
    me:message('칠성당에 입장합니다.', MESSAGE_TYPE.STATE)
    return me:map(10954, math.random(7, 10), 13)
end

local function try_offerings(me, festival_name, offerings, intro)
    if not enterable() then
        me:dialog(nil, '지금은 칠성당을 청소하는 중이니 잠시후에 오십시오.', { prev = false, next = false })
        push_back(me)
        return
    end

    local btn = me:dialog(nil, intro, { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        push_back(me)
        return
    end

    local missing = {}
    for _, name in ipairs(offerings) do
        if not me:has_items(name, 1) then
            missing[#missing + 1] = name
        end
    end

    if #missing > 0 then
        for _, name in ipairs(missing) do
            btn = me:dialog(nil, name_with(name, '을', '를') .. ' 가져오세요.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                break
            end
        end
        push_back(me)
        return
    end

    if not festival.is(festival_name) or not enterable() then
        me:dialog(nil, '지금은 칠성당을 청소하는 중이니 잠시후에 오십시오.', { prev = false, next = false })
        push_back(me)
        return
    end

    local cost = {}
    for _, name in ipairs(offerings) do
        cost[name] = 1
    end
    if not me:rmitem(cost, ITEM_DELETE_TYPE.GIVE) then
        push_back(me)
        return
    end
    for _, name in ipairs(offerings) do
        me:message(name .. ' 주었습니다.', MESSAGE_TYPE.STATE)
    end

    if enter_chilsung(me) ~= true then
        for _, name in ipairs(offerings) do
            me:mkitem(name, 1)
        end
    end
end

function M.try_enter(me)
    if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    if festival.is('설날') then
        enter_chilsung(me)
    elseif festival.is('칠석') then
        try_offerings(me, '칠석', OFFERINGS_CHILSEOK, '고사에 바칠 음식은 가지고 오셨나요? 벼, 호박부침, 잉어구이를 가지고 오셔야 합니다.')
    elseif festival.is('유두') then
        try_offerings(me, '유두', OFFERINGS_YUDU, '고사를 지내시려면 유두벼, 유두콩, 유두조를 구해오셔야 돼요..')
    else
        me:dialog(nil, '지금은 칠성당을 청소하는 중이니 잠시후에 오십시오.', { prev = false, next = false })
        push_back(me)
    end
end

return M
