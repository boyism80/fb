-- @brief   NPC Jung Eoeon (Seollal house): gives 널 during Chunghwa quest; other seasons (e.g. Seollal) not yet implemented.
-- @param[in]  me   The character talking to the NPC.
-- @param[in]  npc  The NPC entity.
-- @note    When gv("sesi_rightnow") == 2 and QUEST_BAEK_MONGYEON step >= 1, gives 널 once.
function NPC_356(me, npc)
    local button = me:dialog(npc, "안녕하세요? 저는 정어언입니다.", false, true)
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

    if me:has_items("널", 1) then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    button = me:dialog(npc, "몽연이가 청소를 한다구요? 그럼 저는 널뛰기 판자를 부탁드릴께요..", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "이 널은 세시마을에 있는 여자애들이 명절때마다 쓰던 널이랍니다. 이젠 너무 낡아서 새로 장만해야겠네요.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "그럼 몽연이에게 잘 전해주세요!", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    me:mkitem("널", 1)
end
