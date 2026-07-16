-- npc: 봉쇄장군
function NPC_473(me, npc)
    ::NPC_473_0001::
    local button = me:dialog(npc, "자네도 우리를 도와주러온 사람인가? 그렇다면 반갑군. 자, 내 말을 잘 듣게.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_473_0002::
    button = me:dialog(npc, "병사들의 이야기를 들어보면 알겠지만, 지금 원숭이들은 먹이를 잘못 먹고는 흉폭해지는 병에 걸려있다네.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_473_0001
    end

    ::NPC_473_0003::
    button = me:dialog(npc, "다행히 전염병이 아니어서 아직 정상인 원숭이들도 많지만, 흉폭해진 원숭이들에 비하면 형평없이 약해서 도망다닐 뿐 이라네.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_473_0002
    end

    ::NPC_473_0004::
    button = me:dialog(npc, "흉폭해진 원숭이들은 치료를 해줄 수도 없어. 다만 처단하는 것만이 해결책이야. 들어가서 용감히 싸워주면 고맙겠군.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_473_0003
    end

    ::NPC_473_0005::
    button = me:dialog(npc, "다만 조심하시게 원숭이들은 머리도 좋고, 힘도 굉장히 강하니까 말이야. 일국의 장군인 나도 몇 번이나 당할 뻔 했지.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_473_0004
    end

    ::NPC_473_0006::
    local sel, btn = me:list(npc, "잘 마른 갈대 10개를 10000전에 팔겠네, 어떤가?", { "네, 어서 주세요", "아뇨, 필요없어요" }, true)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_473_0005
    end
    if sel == 2 then
        return
    end
    if me:money() < 10000 then
        me:dialog(npc, "돈이 없군.", false, false)
        return
    end
    me:money(me:money() - 10000)
    me:mkitem("마른갈대", 10)
    me:dialog(npc, "여기 마른 갈대 10개요.", false, false)
end