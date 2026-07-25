-- npc: 호위무사
local REQUIRED_ITEMS = {
    '해골왕의뼈',
    '유성지의보패',
    '하선녀의실타래',
    '불의수정',
}

local WARP_MAP = '파괴왕의제단'
local WARP_X_MIN, WARP_X_MAX = 13, 16
local WARP_Y_MIN, WARP_Y_MAX = 35, 38

return {
    ON_CLICK = function(me, npc)
        local btn = me:dialog(npc, '파괴왕의방으로 가기 위해선 해골왕의뼈, 유성지의보패, 하선녀의실타래, 불의수정이 필요하다네.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local required_table = {}
        for _, name in ipairs(REQUIRED_ITEMS) do
            required_table[name] = 1
        end
        if not me:has_items(required_table) then
            me:dialog(npc, '자네는 재료가 부족하군..', { prev = false, next = false })
            return
        end
        btn = me:dialog(npc, '재료를 모두 가지고 있군. 그럼 파괴왕의방으로 보내주겠네.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local map = name2map(WARP_MAP)
        if map == nil then
            me:dialog(npc, '존재하지 않는 맵입니다.', { prev = false, next = false })
            return
        end
        local x = math.random(WARP_X_MIN, WARP_X_MAX)
        local y = math.random(WARP_Y_MIN, WARP_Y_MAX)
        me:map(map, { x, y })
    end
}
