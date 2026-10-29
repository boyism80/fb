local festival = require('lib.festival')
local bridge = require('lib.magpie_bridge')

local HOUSE_GYEONU = '견우의집'
local HOUSE_JIKNYEO = '직녀의집'
local SPAWN_JIKNYEO_X = 24
local SPAWN_GYEONU_X = 80
local CAP_JIKNYEO_X = 51
local CAP_GYEONU_X = 53
local MEET_X = 52
local LINE_INTERVAL_MS = 2000
local TEST_WALK_WITHOUT_SUPPORT = false

local MOB_STATUE = 60008
local MOB_GYEONU = 60005
local MOB_JIKNYEO = 60006
local MOB_COUPLE = 60007
local NPC_BAEK = 502

local MOB_NAME = {
    [MOB_STATUE] = '오작교석상',
    [MOB_GYEONU] = '견우',
    [MOB_JIKNYEO] = '직녀',
    [MOB_COUPLE] = '견우직녀',
}

local EVENT_MOB_IDS = {
    [MOB_GYEONU] = true,
    [MOB_JIKNYEO] = true,
    [MOB_STATUE] = true,
    [MOB_COUPLE] = true,
}

local BAEK_LINES = {
    '기. 제.',
    '올 한해도 수해해 주신 조상님이시어',
    '이제 여기 한잔의 술과 각종 음식으로',
    '조상님의 덕을 되새기오니',
    '부디 기꺼운 마음으로 흠향하시옵소서.',
}

local MEET_LINES = {
    { id = MOB_GYEONU, text = '오.. 직녀 나의 꽃이여, 나의 빛이여.' },
    { id = MOB_JIKNYEO, text = '견우님... 저의 삶이자 희망이시여.' },
    { id = MOB_GYEONU, text = '오늘 하루를 기다리는 순간순간이 내게는 형극의 시간이었소.' },
    { id = MOB_JIKNYEO, text = '베 한올한올마다 견우님의 얼굴이 아롱아롱 맺혔답니다.' },
    { id = MOB_GYEONU, text = '비록 잠시나마 그대의 숨결을 느껴보고 싶었소.' },
    { id = MOB_JIKNYEO, text = '저 또한 한시도 견우님을 잊어본 적이 없답니다.' },
    { id = MOB_GYEONU, text = '이리오구려..' },
    { id = MOB_JIKNYEO, text = '네..' },
}

local DEFAULTS = {
    check = 0,
    gyeonu_failed = 0,
    jiknyeo_failed = 0,
    gyeonu_talk = 0,
    jiknyeo_talk = 0,
    couple_tick = 0,
    couple_buff_index = 1,
    countdown_active = false,
    chilsung_countdown = false,
    reunited = false,
    warped = false,
    baek_running = false,
    baek_done = false,
    meet_running = false,
    settle_key = '',
    clock_stamp = 0,
    bridge_dirty = false,
    settle_mobs_cleared = false,
}

local function reset(sm)
    for key, value in pairs(DEFAULTS) do
        sm:set(key, value)
    end
    sm:set('bridge_build_start_sod', nil)
end

local function walk_without_support()
    local v = property('magpie_bridge.walk_without_support')
    if v ~= nil then
        return v == true
    end
    return TEST_WALK_WITHOUT_SUPPORT
end

local function spawn_event_mob(map, mob_id, x, y, direction)
    local mob = map:spawn_mob(mob_id, x, y, direction)
    if mob ~= nil then
        return mob
    end

    local name = MOB_NAME[mob_id]
    if name == nil then
        return nil
    end

    mob = map:spawn_mob(name, x, y, direction)
    if mob ~= nil then
        return mob
    end

    for _, ch in pairs(map:objects(OBJECT_TYPE.CHARACTER)) do
        mob = ch:spawn_mob(name, x, y, false, true)
        if mob ~= nil then
            if direction ~= nil then
                mob:direction(direction)
            end
            return mob
        end
        break
    end
    return nil
end

local function delete_mobs_by_id(map, mob_id)
    if map == nil then
        return
    end
    -- Snapshot first: destroy() yields and can invalidate pairs() iteration.
    local targets = {}
    for _, mob in pairs(map:objects(OBJECT_TYPE.MOB)) do
        if mob:model():id() == mob_id then
            targets[#targets + 1] = mob
        end
    end
    for _, mob in ipairs(targets) do
        mob:destroy()
    end
end

local function clear_event_mobs(map)
    if map == nil then
        return
    end
    local targets = {}
    for _, mob in pairs(map:objects(OBJECT_TYPE.MOB)) do
        if EVENT_MOB_IDS[mob:model():id()] then
            targets[#targets + 1] = mob
        end
    end
    for _, mob in ipairs(targets) do
        mob:destroy()
    end
end

local function flush_bridge_mobs(sm, map)
    if not sm:get('bridge_dirty') then
        return
    end
    clear_event_mobs(map)
    sm:set('bridge_dirty', false)
end

local function clear_bridge_characters(map)
    for x = bridge.BRIDGE_X1, bridge.BRIDGE_X2 do
        local ch = map:at(x, bridge.BRIDGE_Y, OBJECT_TYPE.CHARACTER)
        if ch ~= nil then
            if ch:gender() == GENDER.MALE then
                ch:map(map, math.random(84, 92), math.random(12, 17))
            else
                ch:map(map, math.random(6, 13), math.random(11, 16))
            end
        end
    end
end

local function find_mob(map, mob_id)
    for _, mob in pairs(map:objects(OBJECT_TYPE.MOB)) do
        if mob:model():id() == mob_id then
            return mob
        end
    end
    return nil
end

local function at_cap_for_direction(x, direction)
    if direction == DIRECTION.LEFT then
        return x <= CAP_GYEONU_X
    elseif direction == DIRECTION.RIGHT then
        return x >= CAP_JIKNYEO_X
    else
        return false
    end
end

local function has_bridge_support(map, x, y)
    if walk_without_support() then
        return true
    end
    return map:at(x, y + 2, OBJECT_TYPE.CHARACTER) ~= nil
        or map:at(x, y + 2, OBJECT_TYPE.OBJECT) ~= nil
end

local function partners_ready(gyeonu, jiknyeo)
    if gyeonu == nil or jiknyeo == nil then
        return false
    end

    local gx, gy = gyeonu:position()
    local jx, jy = jiknyeo:position()
    if gy ~= bridge.BRIDGE_Y or jy ~= bridge.BRIDGE_Y then
        return false
    end
    if math.abs(gx - jx) ~= 2 then
        return false
    end
    return gx <= CAP_GYEONU_X and jx >= CAP_JIKNYEO_X
end

local function try_walk(sm, map, mob, direction, lines, failed_key, talk_key)
    if mob == nil then
        return false
    end

    local x, y = mob:position()
    local moved = false
    if not at_cap_for_direction(x, direction) then
        if has_bridge_support(map, x, y) then
            moved = mob:move(direction) == true
        else
            local failed = sm:get(failed_key) + 1
            sm:set(failed_key, failed)
            if failed > 3 then
                mob:chat(lines.fail)
                sm:set(failed_key, 0)
                return true
            end
        end
    end

    local talk = sm:get(talk_key) + 1
    sm:set(talk_key, talk)
    if talk > 3 and moved then
        local pool = lines.walk
        mob:chat(pool[math.random(1, #pool)])
        sm:set(talk_key, 0)
        return true
    end
    return false
end

local function step_couple(sm, map)
    local couple = find_mob(map, MOB_COUPLE)
    if couple == nil then
        return
    end

    local couple_tick = sm:get('couple_tick') + 1
    sm:set('couple_tick', couple_tick)
    if couple_tick <= 10 then
        return
    end

    local index = sm:get('couple_buff_index') or 1
    local list = {}
    for _, ch in pairs(map:objects(OBJECT_TYPE.CHARACTER)) do
        list[#list + 1] = ch
    end
    if #list == 0 or index > #list then
        sm:set('couple_tick', 0)
        sm:set('couple_buff_index', 1)
        return
    end

    local ch = list[index]
    ch:effect(58)
    ch:buff('견우직녀축복', 7)
    sm:set('couple_buff_index', index + 1)
end

local function run_meet_sequence(sm, map)
    if sm:get('meet_running') or sm:get('reunited') then
        return
    end
    sm:set('meet_running', true)

    for i, entry in ipairs(MEET_LINES) do
        if not festival.is('칠석') then
            break
        end
        local speaker = find_mob(map, entry.id)
        if speaker ~= nil then
            speaker:chat(entry.text)
        end
        if i < #MEET_LINES then
            sleep(LINE_INTERVAL_MS)
        end
    end

    if festival.is('칠석') and not sm:get('reunited') then
        sleep(LINE_INTERVAL_MS)
        spawn_event_mob(map, MOB_COUPLE, MEET_X, bridge.BRIDGE_Y, DIRECTION.BOTTOM)
        delete_mobs_by_id(map, MOB_GYEONU)
        delete_mobs_by_id(map, MOB_JIKNYEO)
        sm:set('reunited', true)
        sm:set('couple_tick', 0)
        sm:set('couple_buff_index', 1)
    end

    sm:set('meet_running', false)
end

local function set_map_char_timers(map, seconds, timer_type)
    if map == nil then
        return
    end
    for _, ch in pairs(map:objects(OBJECT_TYPE.CHARACTER)) do
        ch:timer(seconds, timer_type)
    end
end

local function clear_bridge_timer(sm)
    if not sm:get('countdown_active') then
        return
    end
    set_map_char_timers(id2map(bridge.MAP_ID), 0, TIMER_TYPE.OFF)
    sm:set('countdown_active', false)
end

local function warp_chilsung_all(sm)
    local map = id2map(bridge.CHILSUNG_MAP_ID)
    if map == nil then
        return
    end

    local gyeonu = name2map(HOUSE_GYEONU) or id2map(10958)
    local jiknyeo = name2map(HOUSE_JIKNYEO) or id2map(10959)

    if sm:get('chilsung_countdown') then
        set_map_char_timers(map, 0, TIMER_TYPE.OFF)
        sm:set('chilsung_countdown', false)
    end

    for _, ch in pairs(map:objects(OBJECT_TYPE.CHARACTER)) do
        local house = (ch:gender() == GENDER.MALE) and gyeonu or jiknyeo
        if house ~= nil then
            ch:timer(bridge.BRIDGE_BUILD_SECONDS, TIMER_TYPE.DECREASE)
            ch:map(house, math.random(7, 12), math.random(6, 12))
        end
    end
end

local function settle_key(t)
    return string.format('%04d-%02d-%02d-%02d', t.year or 0, t.month or 0, t.day or 0, t.hour or 0)
end

local function datetime_stamp(t)
    if t == nil then
        return 0
    end
    return ((((((t.year or 0) * 100 + (t.month or 0)) * 100 + (t.day or 0)) * 100 + (t.hour or 0)) * 100 + (t.minute or 0)) * 100 + (t.second or 0))
end

local function sync_clock(sm, t)
    local stamp = datetime_stamp(t)
    local prev = sm:get('clock_stamp')
    if prev > 0 and stamp < prev then
        reset(sm)
        sm:set('bridge_dirty', true)
    end
    sm:set('clock_stamp', stamp)
end

local function begin_settle_window(sm, t)
    local key = settle_key(t)
    if sm:get('settle_key') == key then
        return
    end
    sm:set('settle_key', key)
    sm:set('warped', false)
    sm:set('baek_running', false)
    sm:set('baek_done', false)
    sm:set('meet_running', false)
    sm:set('reunited', false)
    sm:set('check', 0)
    sm:set('countdown_active', false)
    sm:set('chilsung_countdown', false)
    sm:set('bridge_build_start_sod', nil)
    sm:set('gyeonu_failed', 0)
    sm:set('jiknyeo_failed', 0)
    sm:set('gyeonu_talk', 0)
    sm:set('jiknyeo_talk', 0)
    sm:set('bridge_dirty', true)
    sm:set('settle_mobs_cleared', false)
end

local function find_baek(chilsung)
    for _, npc in pairs(chilsung:objects(OBJECT_TYPE.NPC)) do
        if npc:model():id() == NPC_BAEK then
            return npc
        end
    end
    return nil
end

local function run_gosa(sm, t)
    if sm:get('baek_done') or sm:get('baek_running') then
        return
    end
    if bridge.BRIDGE_HOURS[t.hour or 0] ~= true then
        return
    end

    local chilsung = id2map(bridge.CHILSUNG_MAP_ID)
    if chilsung == nil then
        return
    end

    begin_settle_window(sm, t)

    local baek = find_baek(chilsung)
    if baek == nil then
        return
    end

    sm:set('baek_running', true)
    for i, line in ipairs(BAEK_LINES) do
        if not festival.is('칠석') then
            break
        end
        baek:chat(line)
        if i < #BAEK_LINES then
            sleep(LINE_INTERVAL_MS)
        end
    end

    sleep(LINE_INTERVAL_MS)
    if not sm:get('warped') then
        sm:set('bridge_build_start_sod', bridge.seconds_of_day(datetime() or t))
        sm:set('countdown_active', false)
        warp_chilsung_all(sm)
        sm:set('warped', true)
    end
    sm:set('baek_done', true)
    sm:set('baek_running', false)
end

local function run_bridge_build_phase(sm, map, t)
    local rem = bridge.bridge_build_remaining(t)

    if sm:get('check') == 0 and not sm:get('settle_mobs_cleared') then
        clear_event_mobs(map)
        sm:set('settle_mobs_cleared', true)
    end

    if rem ~= nil and rem > 0 and not sm:get('countdown_active') then
        set_map_char_timers(map, rem, TIMER_TYPE.DECREASE)
        sm:set('countdown_active', true)
    end
end

local function run_ceremony(sm, map)
    local check = sm:get('check')
    if check >= 5 then
        if sm:get('reunited') then
            step_couple(sm, map)
        elseif not sm:get('meet_running') then
            local gyeonu = find_mob(map, MOB_GYEONU)
            local jiknyeo = find_mob(map, MOB_JIKNYEO)
            if partners_ready(gyeonu, jiknyeo) then
                run_meet_sequence(sm, map)
            else
                clear_bridge_characters(map)
                try_walk(sm, map, gyeonu, DIRECTION.LEFT, {
                    fail = '아아...',
                    walk = { '직녀...', '어디에 있소...', '보고싶소...' },
                }, 'gyeonu_failed', 'gyeonu_talk')
                try_walk(sm, map, jiknyeo, DIRECTION.RIGHT, {
                    fail = '아아...',
                    walk = { '견우님..', '어디에 계신가요...', '보고싶어요...' },
                }, 'jiknyeo_failed', 'jiknyeo_talk')
            end
        end
        return
    end

    check = check + 1
    sm:set('check', check)

    if check == 1 then
        clear_event_mobs(map)
        spawn_event_mob(map, MOB_STATUE, 23, bridge.BRIDGE_Y, DIRECTION.BOTTOM)
        spawn_event_mob(map, MOB_STATUE, 82, bridge.BRIDGE_Y, DIRECTION.BOTTOM)
    elseif check <= 3 then
        clear_bridge_characters(map)
    elseif check == 4 then
        spawn_event_mob(map, MOB_JIKNYEO, SPAWN_JIKNYEO_X, bridge.BRIDGE_Y, DIRECTION.RIGHT)
        spawn_event_mob(map, MOB_GYEONU, SPAWN_GYEONU_X, bridge.BRIDGE_Y, DIRECTION.LEFT)
        sm:set('gyeonu_failed', 0)
        sm:set('jiknyeo_failed', 0)
        sm:set('gyeonu_talk', 0)
        sm:set('jiknyeo_talk', 0)
        sm:set('reunited', false)
        sm:set('meet_running', false)
        sm:set('couple_buff_index', 1)
        sm:set('check', 5)
    end
end

local function run_bridge(sm, map, t)
    flush_bridge_mobs(sm, map)

    if bridge.BRIDGE_HOURS[t.hour or 0] ~= true then
        clear_bridge_timer(sm)
        if sm:get('check') ~= 0 or sm:get('countdown_active') or sm:get('warped') or sm:get('baek_running') or sm:get('bridge_dirty') then
            clear_event_mobs(map)
            reset(sm)
        end
        return
    end

    begin_settle_window(sm, t)

    local rem = bridge.bridge_build_remaining(t)
    if rem ~= nil and rem > 0 then
        run_bridge_build_phase(sm, map, t)
        flush_bridge_mobs(sm, map)
    elseif not sm:get('baek_done') then
        if sm:get('check') == 0 and not sm:get('settle_mobs_cleared') then
            clear_event_mobs(map)
            sm:set('settle_mobs_cleared', true)
        end
    else
        clear_bridge_timer(sm)
        if not sm:get('meet_running') then
            run_ceremony(sm, map)
        end
    end
end

return {
    on_init = function(sm)
        if id2map(bridge.MAP_ID) == nil and id2map(bridge.CHILSUNG_MAP_ID) == nil then
            return
        end
        reset(sm)
        clear_event_mobs(id2map(bridge.MAP_ID))
        sm:every('tick', 1000, 'on_tick')
    end,

    on_tick = function(sm)
        local map = id2map(bridge.MAP_ID)
        if not festival.is('칠석') then
            if sm:get('check') ~= 0 or sm:get('bridge_dirty') then
                clear_event_mobs(map)
                reset(sm)
            end
            return
        end

        local t = datetime()
        if t == nil then
            return
        end
        sync_clock(sm, t)
        run_gosa(sm, t)

        t = datetime()
        if map ~= nil and t ~= nil then
            run_bridge(sm, map, t)
        end
    end,

    on_bridge_enter = function(sm)
        flush_bridge_mobs(sm, id2map(bridge.MAP_ID))
    end,
}
