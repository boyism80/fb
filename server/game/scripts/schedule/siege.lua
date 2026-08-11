-- 신수성 옥좌공성 (화~금 21:00 시작, 타이머 종료 시 옥좌(20,8) 문파장 승리).
local castle_lib = require('lib.castle')

local TIMER_PATH = 'scripts/schedule/siege.lua'
local THRONE_X, THRONE_Y = 20, 8
local MIN_DURATION_SEC = 1800
local MAX_DURATION_SEC = 3600

local CASTLES = {
    [DIVINE_BEAST.VERMILION_BIRD] = { heart_map = 2000, name = '주작', timer = 'siege_end_vermilion', end_func = 'on_siege_end_vermilion' },
    [DIVINE_BEAST.BLACK_TORTOISE] = { heart_map = 2003, name = '현무', timer = 'siege_end_black', end_func = 'on_siege_end_black' },
    [DIVINE_BEAST.AZURE_DRAGON] = { heart_map = 1997, name = '청룡', timer = 'siege_end_azure', end_func = 'on_siege_end_azure' },
    [DIVINE_BEAST.WHITE_TIGER] = { heart_map = 2041, name = '백호', timer = 'siege_end_white', end_func = 'on_siege_end_white' },
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
        if member:role() == CLAN_ROLE.MASTER and member:name() == ch:name() then
            return clan
        end
    end

    return nil
end

local function start_siege(divine_beast)
    local info = CASTLES[divine_beast]
    if info == nil then
        return false, '알 수 없는 신수입니다.'
    end

    local map = id2map(info.heart_map)
    if map == nil then
        return false, string.format('%s 심장 맵을 찾을 수 없습니다.', info.name)
    end

    local castle = castle(divine_beast)
    if castle == nil then
        return false, string.format('%s 성 정보를 찾을 수 없습니다.', info.name)
    end

    local duration_seconds = math.random(MIN_DURATION_SEC, MAX_DURATION_SEC)
    castle:start_siege(duration_seconds)

    map:cancel_timer(info.timer)
    map:set_timer(duration_seconds * 1000, TIMER_PATH, info.end_func, { once = true, name = info.timer })

    broadcast(string.format('[정보] %s의성 공성전이 시작되었습니다.', info.name), MESSAGE_TYPE.WORLD)
    return true, duration_seconds
end

local function end_siege(divine_beast)
    local info = CASTLES[divine_beast]
    if info == nil then
        return false, '알 수 없는 신수입니다.'
    end

    local map = id2map(info.heart_map)
    if map == nil then
        return false, string.format('%s 심장 맵을 찾을 수 없습니다.', info.name)
    end

    local castle = castle(divine_beast)
    if castle == nil then
        return false, string.format('%s 성 정보를 찾을 수 없습니다.', info.name)
    end

    if castle:siege_active() == false then
        return false, string.format('%s의성 공성전이 진행 중이 아닙니다.', info.name)
    end

    local previous_owner_id = castle:owner_clan()

    local occupant = map:at(THRONE_X, THRONE_Y, OBJECT_TYPE.CHARACTER)
    local winner_clan = occupant and clan_as_master(occupant) or nil

    map:cancel_timer(info.timer)
    castle:end_siege(winner_clan and winner_clan:id() or nil)

    if winner_clan ~= nil and winner_clan:id() ~= previous_owner_id then
        if previous_owner_id ~= nil then
            local previous_owner = id2clan(previous_owner_id)
            if previous_owner ~= nil then
                for _, member in pairs(previous_owner:members()) do
                    local online = name2ch(member:name())
                    if online ~= nil then
                        castle_lib.enforce_ownership(online, divine_beast)
                    end
                end
            end
        end

        broadcast(string.format('[정보] %s 문파가 %s의성을 탈환합니다.', winner_clan:name(), info.name), MESSAGE_TYPE.WORLD)
    elseif previous_owner_id ~= nil then
        local owner_clan = id2clan(previous_owner_id)
        local owner_name = owner_clan and owner_clan:name() or ''
        broadcast(string.format('[정보] %s의성이 수성측 %s문파에게 돌아갑니다.', info.name, owner_name), MESSAGE_TYPE.WORLD)
    end

    return true, info.name
end

local function end_active_sieges()
    local ended = {}
    for divine_beast, info in pairs(CASTLES) do
        local entity = castle(divine_beast)
        if entity ~= nil and entity:siege_active() then
            local ok = end_siege(divine_beast)
            if ok then
                table.insert(ended, info.name)
            end
        end
    end
    return ended
end

return {
    NAME_TO_DIVINE_BEAST = NAME_TO_DIVINE_BEAST,
    start_siege = start_siege,
    end_siege = end_siege,
    end_active_sieges = end_active_sieges,

    on_siege_start_vermilion = function() start_siege(DIVINE_BEAST.VERMILION_BIRD) end,
    on_siege_start_black = function() start_siege(DIVINE_BEAST.BLACK_TORTOISE) end,
    on_siege_start_azure = function() start_siege(DIVINE_BEAST.AZURE_DRAGON) end,
    on_siege_start_white = function() start_siege(DIVINE_BEAST.WHITE_TIGER) end,

    on_siege_end_vermilion = function() end_siege(DIVINE_BEAST.VERMILION_BIRD) end,
    on_siege_end_black = function() end_siege(DIVINE_BEAST.BLACK_TORTOISE) end,
    on_siege_end_azure = function() end_siege(DIVINE_BEAST.AZURE_DRAGON) end,
    on_siege_end_white = function() end_siege(DIVINE_BEAST.WHITE_TIGER) end,
}
