function NPC_238(me, npc)
    local button = me:dialog(npc, '시원한 과일 화채를 만들어 드립니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    local items = me:items()
    local count_by_name = {}
    if items ~= nil then
        for _, item in pairs(items) do
            local name = item:model():name()
            count_by_name[name] = (count_by_name[name] or 0) + item:count()
        end
    end

    if (count_by_name['홍옥'] or 0) < 5 or (count_by_name['국광'] or 0) < 5 or
       (count_by_name['깨끗한얼음'] or 0) < 10 or (count_by_name['참수박'] or 0) < 2 then
        me:dialog(npc, '과일 화채의 재료는 홍옥 5개, 국광 5개, 깨끗한얼음 10개, 참수박 2개가 필요합니다.', false, true)
        return
    end

    me:rmitem({['홍옥'] = 5, ['국광'] = 5, ['깨끗한얼음'] = 10, ['참수박'] = 2}, ITEM_DELETE_TYPE.GIVE)
    me:mkitem('과일화채', 1)
    me:dialog(npc, '정말 시원하고 맛있는 과일화채가 여기 나왔습니다.', false, true)
end