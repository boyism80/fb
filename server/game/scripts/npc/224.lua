-- npc: 청룡성문지기
local TOTEM = '청룡'
local TOTEM_KEY = 'dragon'

local function run_gatekeeper(me, npc)
    local selected, button = me:list(npc, '안녕하세요. 어떻게 오셨나요?', { TOTEM .. '성 입장' })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if selected ~= 1 then
        return
    end

    local clan = me:clan()
    if not clan then
        me:dialog(npc, '가입된 문파가 없습니다.')
        return
    end
    if me:state() == STATE.GHOST then
        me:dialog(npc, '유령은 참가할 수 없습니다.')
        return
    end

    local occupant = (_G.clan_castle_occupant or {})[TOTEM_KEY] or ''
    local siege_start = _G.clan_siege_start or 0
    local siege_map = _G.clan_siege_map or ''
    local clan_name = clan:name()
    local castle_name = TOTEM .. '성'
    local map_entrance = name2map(TOTEM .. '성입구')
    local map_inner = name2map(TOTEM .. '의성')

    if not map_entrance then
        me:dialog(npc, '입장할 수 있는 맵이 없습니다.')
        return
    end

    if clan_name == occupant then
        if siege_start == 0 and map_inner then
            me:map(map_inner, math.random(11, 17), math.random(4, 11))
        else
            me:map(map_entrance, math.random(49, 57), math.random(145, 148))
        end
        return
    end

    if siege_map == castle_name then
        me:map(map_entrance, math.random(49, 57), math.random(145, 148))
        return
    end

    me:dialog(npc, string.format('현재 %s 공성이 진행중이지 않습니다.', castle_name))
end

return {
    ON_CLICK = function(me, npc)
        run_gatekeeper(me, npc)
    end
}
