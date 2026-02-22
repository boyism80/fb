function NPC_182(me, npc)
    local ACHIEVEMENT_GHOST = 29
    local quest = me:quest(QUEST_GHOST)

    if quest == nil or quest:step() ~= 2 then
        me:dialog(npc, '안녕하십니까? 저는 마타라고 합니다.', false, true)
        return
    end

    ::NPC_182_COS001::
    local btn = me:dialog(npc, '음... 부적이 더 필요하다고요? 알겠습니다. 마침 그 부적이 조금 남아 있으니 드리지요.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    btn = me:dialog(npc, '그나저나 참 친절한 분이시군요. 생판 모르는 남을 위해 그 머나먼 일본과 이 중국을 왕복하다니. 탄복했습니다.', true, true)
    if btn == DIALOG_RESULT.PREV then
        goto NPC_182_COS001
    end
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if me:mkitem('귀신퇴치부적', 5) == nil then
        me:dialog(npc, '소지품이 가득 차서 귀신퇴치부적을 받을 수 없습니다.', false, true)
        return
    end
    quest:step(3)
    quest:progress(0)
    me:push_achievement(ACHIEVEMENT_GHOST, '귀신퇴치부적을 더 받다.', 7, 1)
end