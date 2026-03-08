-- @brief   NPC Baek Suin (Dongji house): gives 연 during Chunghwa quest; other seasons (e.g. Dongji) not yet implemented.
-- @param[in]  me   The character talking to the NPC.
-- @param[in]  npc  The NPC entity.
-- @note    When gv("sesi_rightnow") == 2 and QUEST_BAEK_MONGYEON step >= 1, gives 연 once.
function NPC_548(me, npc)
    local button = me:dialog(npc, "안녕하세요? 저는 백수인입니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if gv("sesi_rightnow") ~= 2 then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    local quest = me:quest(QUEST_BAEK_MONGYEON)
    if quest == nil then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    if quest:step() < 1 then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    if me:has_items("연", 1) then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    button = me:dialog(npc, "중화절이 되니 몽연이형이 또 세시마을 대청소를 하는군요. 창고에서 또 뭐 꺼내달라고 할 텐데.. 귀찮은데..", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "연을 받아오라고 했다구요? 잠시만요.. 찾아볼께요....", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "............\n\n........여기있네.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "혹시 또 뭐 더 찾아달라고 하면.. 직접와서 좀 찾으라고 해요.. 귀찮아요...", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    me:mkitem("연", 1)
end
