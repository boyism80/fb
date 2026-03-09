
function NPC_540(me, npc)
    local quest = me:quest(QUEST_JINHWANG)
    if quest == nil or quest:step() ~= 5 then
        me:dialog(npc, "에휴... 여행을 다녀왔더니 힘드네요.", false, true)
        return
    end

    ::NPC_540_0000::
    local button = me:dialog(npc, "안녕하세요. 전 방금 영환도사님과 중국 서쪽을 여행하고 오는 길입니다. 목숨이 위험했던 때도 있었지만 정말 신기한 여행을 했답니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_540_0001::
    button = me:dialog(npc, "진황보검이요? 그런건 잘 모르겠는데.. 아! 여행도중 영환도사님이 검을 하나 발견하셨는데 범상치 않은것이라 하셨던 것이 기억나는군요.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_540_0000
    end

    ::NPC_540_0002::
    button = me:dialog(npc, "영환도사님을 찾아가보세요. 곧 또 여행을 떠나실테니 서두르시는게 좋을듯하네요.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_540_0001
    end

    me:push_achievement(511, "진황보검을 찾아서 (영환도사를 만나자)", 7, 20)
    quest:step(6)
end
