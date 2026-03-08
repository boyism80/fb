function NPC_135(me, npc)
    local quest = me:quest(QUEST_SAILOR)
    local btn

    if quest ~= nil and quest:step() == 5 then
        ::NPC_135_COS003::
        btn = me:dialog(npc, '안녕하신가! 여행은 즐거웠나? 난 지금 환상의의 섬으로 한번 더 가 보려고 한다네.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_135_COS004::
        btn = me:dialog(npc, '다녀와서 사람들이 하는 이야기를 듣고서야 알았는데, 그 곳에서만 구할 수 있는 특산품이 굉장히 많더구만! 특히 요리나 장신구가 일품이라고 하네. 뭐, 자네 같은 사람들은 무기나 방어구에나 관심이 있겠지만 말이야. 허허....', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_135_COS003
        end
        quest:step(6)
        return
    end

    if quest == nil or quest:step() ~= 1 then
        me:dialog(npc, '....', false, false)
        return
    end

    ::NPC_135_COS001::
    btn = me:dialog(npc, '또 보게 되는군! 전에도 이야기했지만 난 지금 환상의 섬으로 가는 배편을 알아보러 가는 중이지.\n\n혹시 자네도 환상의 섬으로 가는 길이라면 내 특별히 길을 알려주지.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_135_COS002::
    btn = me:dialog(npc, '환상의 섬으로 가려면 배를 타야 하는데, 남문 밖으로 나가서 남쪽으로 쭉 내려가면 일본선착장이 있을 걸세, 거기서 일본 가는 배를 타면 안되고, 남서쪽으로 조금 더 가면 고균도선착장이란 곳이 나오지! 거기서 배를 타면 환상의섬으로 갈 수 있다네.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_135_COS001
    end

    quest:step(2)
end