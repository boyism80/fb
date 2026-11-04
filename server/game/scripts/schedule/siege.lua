-- 신수성 옥좌공성 (화~금 21:00 시작, 종료 시각에 옥좌(20,8) 문파장 승리).
--
-- Internal owns the siege state. A game host only asks to start, reports the throne result and
-- reacts to the state it receives; the heart map host does all three, the others only read their copy.
--
-- Known limits of this design:
-- - A heart host that is down at the scheduled time starts no siege for that slot. A start that keeps
--   failing is retried only inside the slot.
-- - The announcements and the eviction run when the heart host sees the state change. If it is down at
--   that moment they are skipped, and after a restart the first state read is not treated as a change.
--   The owner and the entry checks stay correct, since they read the state itself.
-- - A lost update reaches the other hosts at the next refresh, so a copy can be late by up to REFRESH_MS.
-- - Every host refreshes all castles every REFRESH_MS, from the state machine.
local castle_lib = require('lib.castle')

local THRONE_X, THRONE_Y = 20, 8
local SCHEDULE_SLOT_SEC = 3600
local SAVE_RETRY_SEC = 10
local START_RETRY_MS = 10000

local CASTLES = {
    [DIVINE_BEAST.VERMILION_BIRD] = { heart_map = 2000, name = '주작' },
    [DIVINE_BEAST.BLACK_TORTOISE] = { heart_map = 2003, name = '현무' },
    [DIVINE_BEAST.AZURE_DRAGON] = { heart_map = 1997, name = '청룡' },
    [DIVINE_BEAST.WHITE_TIGER] = { heart_map = 2041, name = '백호' },
}

local NAME_TO_DIVINE_BEAST = {
    ['주작'] = DIVINE_BEAST.VERMILION_BIRD,
    ['현무'] = DIVINE_BEAST.BLACK_TORTOISE,
    ['청룡'] = DIVINE_BEAST.AZURE_DRAGON,
    ['백호'] = DIVINE_BEAST.WHITE_TIGER,
}

local function clan_as_master(ch)
    local clan = ch:clan()
    if clan == nil then
        return nil
    end

    for _, member in pairs(clan:members()) do
        if member.role == CLAN_ROLE.MASTER and member.name == ch:name() then
            return clan
        end
    end

    return nil
end

local function active_heart_map(info)
    local map = id2map(info.heart_map)
    if map == nil or map:active() ~= true then
        return nil
    end
    return map
end

local function start_siege(divine_beast, siege_key)
    local info = CASTLES[divine_beast]
    if info == nil then
        return false, '알 수 없는 신수입니다.'
    end

    local castle = castle(divine_beast)
    if castle == nil then
        return false, string.format('%s 성 정보를 찾을 수 없습니다.', info.name)
    end

    local started, active, remaining = castle:start_siege(siege_key or (now() * 1000 + math.random(0, 999)))
    if started == nil then
        return false, active, true
    end

    if started then
        return true, remaining
    end

    if active then
        return false, string.format('%s의성 공성전이 이미 진행 중입니다.', info.name)
    end
    return false, string.format('%s의성 공성전을 시작하지 못했습니다.', info.name)
end

local function start_scheduled_siege(divine_beast)
    if active_heart_map(CASTLES[divine_beast]) == nil then
        return
    end

    local slot = now()
    local key = slot - slot % SCHEDULE_SLOT_SEC
    while now() < key + SCHEDULE_SLOT_SEC do
        local _, _, unconfirmed = start_siege(divine_beast, key)
        if unconfirmed ~= true then
            return
        end
        sleep(START_RETRY_MS)
    end
end

local function end_siege(divine_beast)
    local info = CASTLES[divine_beast]
    if info == nil then
        return false, '알 수 없는 신수입니다.'
    end

    local map = active_heart_map(info)
    if map == nil then
        return false, string.format('%s 심장 맵이 이 서버에 없습니다.', info.name)
    end

    local castle = castle(divine_beast)
    if castle == nil then
        return false, string.format('%s 성 정보를 찾을 수 없습니다.', info.name)
    end

    if castle:siege_pending() == false then
        return false, string.format('%s의성 공성전이 진행 중이 아닙니다.', info.name)
    end

    local occupant = map:at(THRONE_X, THRONE_Y, OBJECT_TYPE.CHARACTER)
    local winner_clan = occupant and clan_as_master(occupant) or nil

    local result = castle:end_siege(winner_clan and winner_clan:id() or nil)
    if result == 'not_active' then
        return false, string.format('%s의성 공성전이 진행 중이 아닙니다.', info.name)
    elseif result == 'save_failed' then
        return false, string.format('%s의성 새 성주를 저장하지 못해 잠시 뒤 다시 종료합니다.', info.name), true
    end

    return true, info.name
end

local function on_siege_ended(info, previous_owner, owner)
    if owner ~= 0 and owner ~= previous_owner then
        local winner = id2clan(owner)
        local winner_name = winner and winner:name() or ''
        broadcast(string.format('[정보] %s 문파가 %s의성을 탈환합니다.', winner_name, info.name), MESSAGE_TYPE.WORLD)
    elseif owner ~= 0 then
        local owner_clan = id2clan(owner)
        local owner_name = owner_clan and owner_clan:name() or ''
        broadcast(string.format('[정보] %s의성이 수성측 %s문파에게 돌아갑니다.', info.name, owner_name), MESSAGE_TYPE.WORLD)
    end

    castle_lib.evict_castle_maps(info.name)
end

local function react_to_state(sm)
    for divine_beast, info in pairs(CASTLES) do
        local entity = castle(divine_beast)
        if entity ~= nil and active_heart_map(info) ~= nil then
            local key = tostring(divine_beast)
            local round = entity:siege_id() or 0
            local pending = entity:siege_pending() and 1 or 0
            local owner = entity:owner_clan() or 0
            local seen_round = sm:get('round_' .. key)
            local seen_pending = sm:get('pending_' .. key)
            local seen_owner = sm:get('owner_' .. key)
            if seen_round ~= nil then
                if round ~= seen_round then
                    broadcast(string.format('[정보] %s의성 공성전이 시작되었습니다.', info.name), MESSAGE_TYPE.WORLD)
                    seen_pending = 1
                end
                if seen_pending == 1 and pending == 0 then
                    on_siege_ended(info, seen_owner, owner)
                end
            end

            sm:set('round_' .. key, round)
            sm:set('pending_' .. key, pending)
            if pending == 0 or seen_round == nil then
                sm:set('owner_' .. key, owner)
            end
        end
    end
end

local function end_active_sieges()
    local requested = {}
    local failed = {}
    for divine_beast, info in pairs(CASTLES) do
        local entity = castle(divine_beast)
        if entity ~= nil and entity:siege_pending() then
            local pending, reason = entity:finish_siege()
            if pending == nil then
                table.insert(failed, string.format('%s (%s)', info.name, reason))
            elseif pending then
                table.insert(requested, info.name)
            end
        end
    end
    return requested, failed
end

local function has_heart_map()
    for _, info in pairs(CASTLES) do
        if active_heart_map(info) ~= nil then
            return true
        end
    end
    return false
end

local function tick(sm)
    for divine_beast, info in pairs(CASTLES) do
        local retry_key = 'retry_' .. tostring(divine_beast)
        local retry_at = sm:get(retry_key)
        local entity = castle(divine_beast)
        local due = entity ~= nil and entity:siege_pending() and (entity:siege_end_time() or 0) == 0
        local waiting = retry_at ~= nil and now() < retry_at
        if due and waiting == false and active_heart_map(info) ~= nil then
            local _, _, save_failed = end_siege(divine_beast)
            if save_failed then
                sm:set(retry_key, now() + SAVE_RETRY_SEC)
            else
                sm:set(retry_key, nil)
            end
        end
    end

    react_to_state(sm)
end

return {
    NAME_TO_DIVINE_BEAST = NAME_TO_DIVINE_BEAST,
    start_siege = start_siege,
    end_siege = end_siege,
    end_active_sieges = end_active_sieges,
    has_heart_map = has_heart_map,
    tick = tick,

    on_siege_start_vermilion = function() start_scheduled_siege(DIVINE_BEAST.VERMILION_BIRD) end,
    on_siege_start_black = function() start_scheduled_siege(DIVINE_BEAST.BLACK_TORTOISE) end,
    on_siege_start_azure = function() start_scheduled_siege(DIVINE_BEAST.AZURE_DRAGON) end,
    on_siege_start_white = function() start_scheduled_siege(DIVINE_BEAST.WHITE_TIGER) end,
}
