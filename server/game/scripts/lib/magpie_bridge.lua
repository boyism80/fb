-- Chilseok Magpie Bridge (오작교) realtime event.
--
-- Timer (1s): phase detection + walker steps only.
-- Sleep: dialogue sequences (baek gosa / meet lines). Safe because on_tick
-- holds tick_busy so overlapping repeat callbacks no-op while sleeping.
local festival = require('lib.festival')
local state = require('lib.magpie_bridge_state')

local M = {}

local MAP_ID = 10957
local CHILSUNG_MAP_ID = 10954
local HOUSE_GYEONU = '견우의집'
local HOUSE_JIKNYEO = '직녀의집'
local TIMER_NAME = 'magpie_bridge'
local CHILSUNG_TIMER_NAME = 'magpie_chilsung'
local TIMER_PATH = 'scripts/lib/magpie_bridge.lua'
local BRIDGE_Y = 13
local BRIDGE_X1 = 24
local BRIDGE_X2 = 81
-- Walkers: 직녀@24→RIGHT, 견우@80→LEFT, equal 27 steps, stop with 1-cell gap.
local SPAWN_JIKNYEO_X = 24
local SPAWN_GYEONU_X = 80
local CAP_JIKNYEO_X = 51
local CAP_GYEONU_X = 53
local MEET_X = 52
-- Bridge-building countdown starts at gosa warp (not at hour :00).
local BRIDGE_BUILD_SECONDS = 300
local LINE_INTERVAL_MS = 2000

-- Test flag: when true, walkers move without a character/object on (x, y+2).
-- Runtime override: property('magpie_bridge.walk_without_support', true|false)
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

local BAEK_LINES = {
    '기. 제.',
    '올 한해도 수해해 주신 조상님이시어',
    '이제 여기 한잔의 술과 각종 음식으로',
    '조상님의 덕을 되새기오니',
    '부디 기꺼운 마음으로 흠향하시옵소서.',
}

local BRIDGE_HOURS = {
    [0] = true,
    [6] = true,
    [12] = true,
    [18] = true,
}

-- Alternating meet lines (견우 / 직녀). Played with sleep between them.
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

local function walk_without_support()
    local v = property('magpie_bridge.walk_without_support')
    if v ~= nil then
        return v == true
    end
    return TEST_WALK_WITHOUT_SUPPORT
end

local function seconds_of_day(t)
    if t == nil then
        return 0
    end
    return ((t.hour or 0) * 3600) + ((t.minute or 0) * 60) + (t.second or 0)
end

-- Remaining bridge-build seconds after gosa warp. nil = not started, 0 = finished.
local function bridge_build_remaining(t)
    if not state.get('bridge_build_active') then
        return nil
    end
    t = t or datetime()
    if t == nil then
        return nil
    end
    local start_sod = state.get('bridge_build_start_sod') or 0
    local elapsed = seconds_of_day(t) - start_sod
    if elapsed < 0 then
        elapsed = elapsed + 86400
    end
    local rem = BRIDGE_BUILD_SECONDS - elapsed
    if rem < 0 then
        return 0
    end
    return rem
end

local function bridge_build_finished(t)
    local rem = bridge_build_remaining(t)
    return rem ~= nil and rem <= 0
end

local function mark_event_mob(mob)
    if mob == nil then
        return nil
    end
    -- Event props (견우/직녀/합체/석상): take no damage. Same goal as 백열장수련도우미
    -- HP-restore, but invincible covers physical hits too (training helper only hooks spells).
    mob:invincible(true)
    return mob
end

local function spawn_event_mob(map, mob_id, x, y, direction)
    local mob = map:spawn_mob(mob_id, x, y, direction)
    if mob ~= nil then
        return mark_event_mob(mob)
    end

    local name = MOB_NAME[mob_id]
    if name == nil then
        return nil
    end

    mob = map:spawn_mob(name, x, y, direction)
    if mob ~= nil then
        return mark_event_mob(mob)
    end

    for _, ch in pairs(map:objects(OBJECT_TYPE.CHARACTER)) do
        mob = ch:spawn_mob(name, x, y, false, true)
        if mob ~= nil then
            if direction ~= nil then
                mob:direction(direction)
            end
            return mark_event_mob(mob)
        end
        break
    end
    return nil
end

local EVENT_MOB_IDS = {
    [MOB_GYEONU] = true,
    [MOB_JIKNYEO] = true,
    [MOB_STATUE] = true,
    [MOB_COUPLE] = true,
}

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

-- Must run on 오작교 map thread. Includes fused 견우직녀 (60007).
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

local function flush_bridge_mobs(map)
    if not state.get('bridge_dirty') then
        return
    end
    clear_event_mobs(map)
    state.set('bridge_dirty', false)
end

local function clear_bridge_characters(map)
    for x = BRIDGE_X1, BRIDGE_X2 do
        local ch = map:at(x, BRIDGE_Y, OBJECT_TYPE.CHARACTER)
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
    end
    if direction == DIRECTION.RIGHT then
        return x >= CAP_JIKNYEO_X
    end
    return false
end

local function has_bridge_support(map, x, y)
    if walk_without_support() then
        return true
    end
    return map:at(x, y + 2, OBJECT_TYPE.CHARACTER) ~= nil
        or map:at(x, y + 2, OBJECT_TYPE.OBJECT) ~= nil
end

local function partners_ready(map, gyeonu, jiknyeo)
    if gyeonu == nil or jiknyeo == nil then
        return false
    end

    local gx, gy = gyeonu:position()
    local jx, jy = jiknyeo:position()
    if gy ~= BRIDGE_Y or jy ~= BRIDGE_Y then
        return false
    end
    if math.abs(gx - jx) ~= 2 then
        return false
    end
    return gx <= CAP_GYEONU_X and jx >= CAP_JIKNYEO_X
end

local function try_walk(map, mob, direction, lines_search, failed_key, talk_key)
    if mob == nil then
        return false
    end

    local x, y = mob:position()
    local moved = false
    if not at_cap_for_direction(x, direction) then
        if has_bridge_support(map, x, y) then
            moved = mob:move(direction) == true
        else
            local failed = state.bump(failed_key)
            if failed > 3 then
                mob:chat(lines_search.fail)
                state.set(failed_key, 0)
                return true
            end
        end
    end

    local talk = state.bump(talk_key)
    if talk > 3 and moved then
        local pool = lines_search.walk
        mob:chat(pool[math.random(1, #pool)])
        state.set(talk_key, 0)
        return true
    end
    return false
end

local function step_couple(map)
    local couple = find_mob(map, MOB_COUPLE)
    if couple == nil then
        return
    end

    if state.bump('couple_tick') <= 10 then
        return
    end

    local index = state.get('couple_buff_index') or 1
    local list = {}
    for _, ch in pairs(map:objects(OBJECT_TYPE.CHARACTER)) do
        list[#list + 1] = ch
    end
    if #list == 0 or index > #list then
        state.set('couple_tick', 0)
        state.set('couple_buff_index', 1)
        return
    end

    local ch = list[index]
    ch:effect(58)
    ch:buff('견우직녀축복', 7)
    state.set('couple_buff_index', index + 1)
end

local function run_meet_sequence(map)
    if state.get('meet_running') or state.get('reunited') then
        return
    end
    state.set('meet_running', true)

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

    if festival.is('칠석') and not state.get('reunited') then
        sleep(LINE_INTERVAL_MS)
        spawn_event_mob(map, MOB_COUPLE, MEET_X, BRIDGE_Y, DIRECTION.BOTTOM)
        delete_mobs_by_id(map, MOB_GYEONU)
        delete_mobs_by_id(map, MOB_JIKNYEO)
        state.set('reunited', true)
        state.set('couple_tick', 0)
        state.set('couple_buff_index', 1)
    end

    state.set('meet_running', false)
end

-- Per-character countdown on this map only (map:objects runs on map thread).
local function set_map_char_timers(map, seconds)
    if map == nil then
        return
    end
    for _, ch in pairs(map:objects(OBJECT_TYPE.CHARACTER)) do
        ch:timer(seconds, true)
    end
end

local function clear_bridge_timer()
    if not state.get('countdown_active') then
        return
    end
    set_map_char_timers(game_map(MAP_ID), 0)
    state.set('countdown_active', false)
end

local function start_bridge_build(t)
    t = t or datetime()
    state.set('bridge_build_active', true)
    state.set('bridge_build_start_sod', seconds_of_day(t))
    state.set('countdown_active', false)
    -- Per-character timers: warp_chilsung_all (칠성당), sync_timer (house/bridge enter),
    -- and on_tick for anyone already on 오작교.
end

-- Must run on 칠성당 thread.
local function warp_chilsung_all()
    local map = game_map(CHILSUNG_MAP_ID)
    if map == nil then
        return
    end

    local gyeonu = game_map(HOUSE_GYEONU) or game_map(10958)
    local jiknyeo = game_map(HOUSE_JIKNYEO) or game_map(10959)

    if state.get('chilsung_countdown') then
        set_map_char_timers(map, 0)
        state.set('chilsung_countdown', false)
    end

    for _, ch in pairs(map:objects(OBJECT_TYPE.CHARACTER)) do
        local house = (ch:gender() == GENDER.MALE) and gyeonu or jiknyeo
        if house ~= nil then
            ch:timer(BRIDGE_BUILD_SECONDS, true)
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

-- /현재시간 can rewind into the same settle hour; baek_done would stick forever
-- if we only keyed by year-month-day-hour. Reset ceremony state on rewind.
local function sync_clock(t)
    local stamp = datetime_stamp(t)
    local prev = state.get('clock_stamp') or 0
    if prev > 0 and stamp < prev then
        state.reset()
        -- Mobs live on 오작교; clear from on_tick / on_map_enter (bridge thread).
        state.set('bridge_dirty', true)
    end
    state.set('clock_stamp', stamp)
end

-- New 0/6/12/18 window: clear leftover flags from previous tests/runs.
local function begin_settle_window(t)
    local key = settle_key(t)
    if state.get('settle_key') == key then
        return
    end
    state.set('settle_key', key)
    state.set('warped', false)
    state.set('baek_running', false)
    state.set('baek_started', false)
    state.set('baek_done', false)
    state.set('meet_running', false)
    state.set('reunited', false)
    state.set('check', 0)
    state.set('countdown_active', false)
    state.set('chilsung_countdown', false)
    state.set('bridge_build_active', false)
    state.set('bridge_build_start_sod', 0)
    state.set('gyeonu_failed', 0)
    state.set('jiknyeo_failed', 0)
    state.set('gyeonu_talk', 0)
    state.set('jiknyeo_talk', 0)
    -- property persists across script reloads; a crashed tick can leave this stuck.
    state.set('tick_busy', false)
    state.set('bridge_dirty', true)
    state.set('settle_mobs_cleared', false)
end

local function find_baek(chilsung)
    if chilsung == nil then
        return nil
    end
    for _, npc in pairs(chilsung:objects(OBJECT_TYPE.NPC)) do
        if npc:model():id() == NPC_BAEK then
            return npc
        end
    end
    return nil
end

local function run_bridge_build_phase(map, t)
    begin_settle_window(t)
    local rem = bridge_build_remaining(t)

    if state.get('check') == 0 and not state.get('settle_mobs_cleared') then
        clear_event_mobs(map)
        state.set('settle_mobs_cleared', true)
    end

    if rem ~= nil and rem > 0 and not state.get('countdown_active') then
        set_map_char_timers(map, rem)
        state.set('countdown_active', true)
    end
end

-- Runs on 칠성당 map thread every 1s.
function M.on_chilsung_tick()
    if not festival.is('칠석') then
        return
    end

    local t = datetime()
    if t == nil then
        return
    end
    sync_clock(t)

    if state.get('baek_done') or state.get('baek_running') then
        return
    end
    if BRIDGE_HOURS[t.hour or 0] ~= true then
        return
    end

    local chilsung = game_map(CHILSUNG_MAP_ID)
    if chilsung == nil then
        return
    end

    begin_settle_window(t)

    local baek = find_baek(chilsung)
    if baek == nil then
        return
    end

    state.set('baek_running', true)
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
    if not state.get('warped') then
        local now = datetime() or t
        start_bridge_build(now)
        warp_chilsung_all()
        state.set('warped', true)
    end
    state.set('baek_done', true)
    state.set('baek_running', false)
end

local function run_ceremony(map)
    local check = state.get('check')
    if check >= 5 then
        if state.get('reunited') then
            step_couple(map)
            return
        end
        if state.get('meet_running') then
            return
        end

        local gyeonu = find_mob(map, MOB_GYEONU)
        local jiknyeo = find_mob(map, MOB_JIKNYEO)
        if partners_ready(map, gyeonu, jiknyeo) then
            run_meet_sequence(map)
            return
        end

        -- Both must step every tick. Returning after 견우 chat skipped 직녀
        -- and made 견우 reach the meet point first.
        try_walk(map, gyeonu, DIRECTION.LEFT, {
            fail = '아아...',
            walk = { '직녀...', '어디에 있소...', '보고싶소...' },
        }, 'gyeonu_failed', 'gyeonu_talk')
        try_walk(map, jiknyeo, DIRECTION.RIGHT, {
            fail = '아아...',
            walk = { '견우님..', '어디에 계신가요...', '보고싶어요...' },
        }, 'jiknyeo_failed', 'jiknyeo_talk')
        return
    end

    check = state.bump('check')

    if check == 1 then
        clear_event_mobs(map)
        spawn_event_mob(map, MOB_STATUE, 23, BRIDGE_Y, DIRECTION.BOTTOM)
        spawn_event_mob(map, MOB_STATUE, 82, BRIDGE_Y, DIRECTION.BOTTOM)
        return
    end

    if check >= 2 and check <= 3 then
        clear_bridge_characters(map)
        return
    end

    if check == 4 then
        spawn_event_mob(map, MOB_JIKNYEO, SPAWN_JIKNYEO_X, BRIDGE_Y, DIRECTION.RIGHT)
        spawn_event_mob(map, MOB_GYEONU, SPAWN_GYEONU_X, BRIDGE_Y, DIRECTION.LEFT)
        state.set('gyeonu_failed', 0)
        state.set('jiknyeo_failed', 0)
        state.set('gyeonu_talk', 0)
        state.set('jiknyeo_talk', 0)
        state.set('reunited', false)
        state.set('meet_running', false)
        state.set('couple_buff_index', 1)
        state.set('check', 5)
    end
end

function M.bridge_build_remaining(t)
    return bridge_build_remaining(t)
end

-- Compat for callers that still pass minute/second; prefers live build timer.
function M.settle_remaining(minute, second)
    local rem = bridge_build_remaining()
    if rem ~= nil then
        return rem
    end
    if minute == nil then
        return 0
    end
    return 0
end

local function ensure_bridge_timer()
    local map = game_map(MAP_ID)
    if map == nil then
        return false
    end
    local id = map:set_timer(1000, TIMER_PATH, 'on_tick', { name = TIMER_NAME })
    return id ~= nil
end

local function ensure_chilsung_timer()
    local chilsung = game_map(CHILSUNG_MAP_ID)
    if chilsung == nil then
        return false
    end
    local id = chilsung:set_timer(1000, TIMER_PATH, 'on_chilsung_tick', {
        name = CHILSUNG_TIMER_NAME,
    })
    return id ~= nil
end

function M.start(map)
    map = map or game_map(MAP_ID)
    if map == nil then
        return false
    end
    state.reset()
    state.set('tick_busy', false)
    clear_event_mobs(map)
    local ok = ensure_bridge_timer()
    ensure_chilsung_timer()
    return ok
end

-- Start/replace drivers without wiping ceremony state.
function M.ensure_timer()
    state.set('tick_busy', false)
    local ok = ensure_bridge_timer()
    ensure_chilsung_timer()
    return ok
end

function M.on_bridge_enter(me)
    M.ensure_timer()
    M.sync_timer(me)
    local map = (me ~= nil) and me:map() or game_map(MAP_ID)
    flush_bridge_mobs(map)
end

function M.sync_timer(me)
    if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    if not festival.is('칠석') then
        return
    end
    local t = datetime()
    if t == nil or BRIDGE_HOURS[t.hour or 0] ~= true then
        return
    end
    local rem = bridge_build_remaining(t)
    if rem == nil or rem <= 0 then
        return
    end
    me:timer(rem, true)
end

function M.on_tick()
    local map = game_map(MAP_ID)
    if map == nil then
        return
    end

    if not festival.is('칠석') then
        if state.get('check') ~= 0 or state.get('bridge_dirty') then
            clear_event_mobs(map)
            state.reset()
        end
        return
    end

    local t = datetime()
    if t == nil then
        return
    end
    sync_clock(t)
    flush_bridge_mobs(map)

    if BRIDGE_HOURS[t.hour or 0] ~= true then
        clear_bridge_timer()
        if state.get('check') ~= 0 or state.get('countdown_active') or state.get('warped') or state.get('baek_running') or state.get('bridge_dirty') then
            clear_event_mobs(map)
            state.reset()
        end
        return
    end

    begin_settle_window(t)

    -- After gosa warp: build bridge until the 5-minute timer ends.
    if state.get('bridge_build_active') and not bridge_build_finished(t) then
        run_bridge_build_phase(map, t)
        flush_bridge_mobs(map)
        return
    end

    -- Waiting for gosa (timer not started yet).
    if not state.get('baek_done') then
        if state.get('check') == 0 and not state.get('settle_mobs_cleared') then
            clear_event_mobs(map)
            state.set('settle_mobs_cleared', true)
        end
        return
    end

    -- Ceremony: kill countdown UI once, then 견우/직녀 march.
    clear_bridge_timer()
    if state.get('meet_running') then
        return
    end
    run_ceremony(map)
end

function M.on_step(me)
    if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    if not festival.is('칠석') then
        return
    end

    local t = datetime()
    if t == nil or BRIDGE_HOURS[t.hour or 0] ~= true then
        return
    end
    local rem = bridge_build_remaining(t)
    if rem == nil or rem <= 0 then
        return
    end

    local map = me:map()
    if map == nil or map:model():id() ~= MAP_ID then
        return
    end

    local x, y = me:position()
    if y ~= BRIDGE_Y or x < BRIDGE_X1 or x > BRIDGE_X2 then
        return
    end

    if map:at(x, y + 2) == nil then
        me:map(map, x, y + 2)
    end
end

return M
