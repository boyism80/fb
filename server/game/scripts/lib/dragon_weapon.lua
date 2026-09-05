local server = require('lib.server')

local M = {}

local DRAGON_WEAPON_NAMES = {
    ['용마제일검'] = true,
    ['용마제이검'] = true,
    ['용마제삼검'] = true,
    ['용마제사검'] = true,
    ['용마제오검'] = true,
    ['용마제육검'] = true,
    ['용마제칠검'] = true,
    ['용마제팔검'] = true,
    ['용마제구검'] = true,
    ['용천제일검'] = true,
    ['용천제이검'] = true,
    ['용천제삼검'] = true,
    ['용천제사검'] = true,
    ['용천제오검'] = true,
    ['용천제육검'] = true,
    ['용천제칠검'] = true,
    ['용천제팔검'] = true,
    ['용천제구검'] = true,
    ['용랑제일봉'] = true,
    ['용랑제이봉'] = true,
    ['용랑제삼봉'] = true,
    ['용랑제사봉'] = true,
    ['용랑제오봉'] = true,
    ['용랑제육봉'] = true,
    ['용랑제칠봉'] = true,
    ['용랑제팔봉'] = true,
    ['용랑제구봉'] = true,
    ['용겸제일봉'] = true,
    ['용겸제이봉'] = true,
    ['용겸제삼봉'] = true,
    ['용겸제사봉'] = true,
    ['용겸제오봉'] = true,
    ['용겸제육봉'] = true,
    ['용겸제칠봉'] = true,
    ['용겸제팔봉'] = true,
    ['용겸제구봉'] = true,
    ['[손상]용마제칠검'] = true,
    ['[손상]용마제팔검'] = true,
    ['[손상]용천제칠검'] = true,
    ['[손상]용천제팔검'] = true,
    ['[손상]용랑제칠봉'] = true,
    ['[손상]용랑제팔봉'] = true,
    ['[손상]용겸제칠봉'] = true,
    ['[손상]용겸제팔봉'] = true,
}

function M.has_dragon_weapon(me)
    local weapon = me:weapon()
    if weapon ~= nil and DRAGON_WEAPON_NAMES[weapon:model():name()] then
        return true
    end

    for _, item in pairs(me:items()) do
        if DRAGON_WEAPON_NAMES[item:model():name()] then
            return true
        end
    end

    return false
end

function M.on_lair_move(me)
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    if not M.has_dragon_weapon(me) then
        return
    end

    server.warp_to_return_map(me)
end

function M.on_king_kill(killer, mobs, weapon_name)
    if killer == nil or not killer:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    if mobs == nil or #mobs == 0 then
        return
    end

    local map = killer:map()
    if map == nil then
        return
    end

    local characters = {}
    for _, ch in pairs(map:objects(OBJECT_TYPE.CHARACTER)) do
        table.insert(characters, ch)
    end

    if killer:mkitem(weapon_name, 1) == nil then
        killer:message('소지품이 가득 차서 ' .. name_with(weapon_name, '을', '를') .. ' 받을 수 없습니다.')
    end

    local killer_oid = killer:oid()
    for _, ch in ipairs(characters) do
        if ch:oid() ~= killer_oid then
            server.warp_to_return_map(ch)
        end
    end
    server.warp_to_return_map(killer)
end

return M
