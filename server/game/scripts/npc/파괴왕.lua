local SUMMON_ITEMS = {
    '해골왕의뼈',
    '유성지의보패',
    '불의수정',
    '하선녀의실타래',
}

function NPC_93(me, npc)
    local sel = me:list(npc, '....', { '파괴왕 소환' }, false)
    if sel == nil or sel ~= 0 then
        return
    end
    local items = {}
    for _, name in ipairs(SUMMON_ITEMS) do
        items[name] = 1
    end
    if not me:has_items(items) then
        me:dialog(npc, '소환에 필요한 재료가 부족합니다.', false, false)
        return
    end
    me:rmitem(items, ITEM_DELETE_TYPE.GIVE)
    local x, y = npc:position()
    npc:destroy()
    me:spawn_mob('파괴왕', x, y, false)
end