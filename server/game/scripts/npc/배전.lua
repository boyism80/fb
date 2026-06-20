local quest = require('lib.quest')
function NPC_32(me, npc)
    local main_q = me:quest(quest.QUEST_SKULL_NECKLACE)
    if main_q == nil or main_q:step() ~= 10 then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    ::NPC_32_0001::
    local b = me:dialog(npc, "뭐? 문자를 배울 수 있는 책? 창힐독본말인가? 음, 그건 왕실 학문관에서만 볼 수 있는 책일세", false, true)
    if b == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_32_0002::
    local sel, btn = me:list(npc, "아무리 필요해도 빼올 수는 없어.", { "어떻게든 수를 내주세요.", "그래요? 아쉽네요." }, true)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_32_0001
    end
    if sel ~= 0 then
        return
    end
    ::NPC_32_0003::
    b = me:dialog(npc, "내가 손주에게 보여주려고 빌려둔 책이 두어권 있는데, 이걸 자네에게 주겠네.", true, true)
    if b == DIALOG_RESULT.QUIT then
        return
    end
    if b == DIALOG_RESULT.PREV then
        goto NPC_32_0002
    end
    ::NPC_32_0004::
    b = me:dialog(npc, "다만, 책을 분실하면 왕실 학문관에 권당 1만전의 벌금을 물어야 한다네.", true, true)
    if b == DIALOG_RESULT.QUIT then
        return
    end
    if b == DIALOG_RESULT.PREV then
        goto NPC_32_0003
    end
    ::NPC_32_0005::
    sel, btn = me:list(npc, "자네가 대신 물어줄 수 있겠나?", { "음, 대신 물어드릴께요.", "그럴 순 없죠." }, true)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_32_0004
    end
    if sel ~= 0 then
        return
    end
    if me:money() < 20000 then
        me:dialog(npc, "물어줄 돈이 없구만. 썩 물러나게.", false, false)
        return
    end
    me:money(me:money() - 20000)
    me:mkitem("창힐독본", 2)
    main_q:step(11)
    me:dialog(npc, "수고했네.", false, false)
end
