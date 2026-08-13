-- Yudu 칠성당 고사 (정각 감지=timer, 대사 간격=sleep).
local festival = require('lib.festival')
local npc_speak = require('lib.npc_speak')

local M = {}

local MAP_ID = 10954
local TIMER_NAME = 'yudu_gosa'
local TIMER_PATH = 'scripts/lib/yudu_gosa.lua'
local NPC_BAEK = 502
local ACHIEVEMENT_GOSA = 561

local LINES = {
    '기. 제.',
    '모든 부정한 것들을 물리치고',
    '정한 마음가짐과 행동을 하오니',
    '부디 천신께서도 굽어살피시어',
    '모든 액으로부터 수호하시옵소서.',
}

local LINE_INTERVAL_MS = 5000
local REWARDS = { '수단', '건단', '상화병', '밀전병' }

local PROP_RUNNING = 'sesi.yudu_gosa_running'
local PROP_HOUR = 'sesi.yudu_gosa_hour'

local function hour_key(t)
    return string.format('%04d-%02d-%02d-%02d', t.year or 0, t.month or 0, t.day or 0, t.hour or 0)
end

local function run_ceremony(map)
    if property(PROP_RUNNING) then
        return
    end
    property(PROP_RUNNING, true)

    npc_speak.lines(map, NPC_BAEK, LINES, LINE_INTERVAL_MS, function()
        return festival.is('유두')
    end, '백남인')

    property(PROP_RUNNING, false)
end

function M.on_tick()
    if not festival.is('유두') then
        property(PROP_HOUR, nil)
        return
    end

    local map = id2map(MAP_ID)
    if map == nil then
        return
    end

    local t = datetime()
    if t == nil then
        return
    end
    if not festival.is_triple_hour(t.hour or 0) or (t.minute or 0) ~= 0 then
        return
    end

    local key = hour_key(t)
    if property(PROP_HOUR) == key or property(PROP_RUNNING) then
        return
    end
    property(PROP_HOUR, key)
    run_ceremony(map)
end

function M.start(map)
    map = map or id2map(MAP_ID)
    if map == nil then
        return false
    end
    map:cancel_timer(TIMER_NAME)
    local id = map:set_timer(1000, TIMER_PATH, 'on_tick', { name = TIMER_NAME })
    return id ~= nil
end

function M.try_reward(me, npc)
    if me == nil then
        return false
    end
    if not festival.is('유두') then
        return false
    end

    local t = datetime()
    if t == nil then
        return true
    end
    if not festival.is_triple_hour(t.hour or 0) then
        me:dialog(npc, '지금은 제를 올리는 시간이 아닙니다...', { prev = false, next = false })
        return true
    end
    if property(PROP_RUNNING) then
        me:dialog(npc, '이 앞으로 나오지 마십시오.', { prev = false, next = false })
        return true
    end

    local reward = REWARDS[math.random(1, #REWARDS)]
    local btn = me:dialog(npc, '자.. 그럼 고사음식을 나눠드리겠습니다.', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    btn = me:dialog(npc, name_with(reward, '을', '를') .. ' 드릴게요.. 맛있게 드세요~~', { prev = false, next = true })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end
    if me:mkitem(reward, 1) == nil then
        me:dialog(npc, '소지품이 가득 차서 드릴 수 없습니다.', { prev = false, next = false })
        return true
    end
    me:message(reward .. ' 받았습니다.', MESSAGE_TYPE.NOTIFY)

    local count_key = 'sesi.yudu_gosa_count.' .. me:name()
    local count = (property(count_key) or 0) + 1
    property(count_key, count)
    me:push_achievement(ACHIEVEMENT_GOSA, string.format('백남인에게 고사음식을 %d번 받았다.', count), 7, 1)

    local village = id2map(10932) or name2map('세시마을')
    if village ~= nil then
        me:map(village, math.random(52, 59), math.random(46, 50))
    end
    return true
end

return M
