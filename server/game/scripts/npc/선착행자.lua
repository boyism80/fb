function NPC_136(me, npc)
    local quest = me:quest(QUEST_SAILOR)
    local btn

    if quest ~= nil and quest:step() == 6 then
        ::NPC_136_COS003::
        btn = me:dialog(npc, '하하, 환상의 섬은 정말 즐거운 곳이었어. 그렇지 않나? 난 지금 그곳에서만 구할 수 있다는 특산품을 사러 가는 중일세.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_136_COS004::
        btn = me:dialog(npc, '특히 그곳에서만 구할 수 있다는 요리를 이야기를 들으니 이거 먹고 싶어서 참을 수가 없더구만. 허허...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_136_COS003
        end
        quest:step(7)
        return
    end

    if quest == nil or quest:step() ~= 2 then
        me:dialog(npc, '....', false, false)
        return
    end

    ::NPC_136_COS001::
    btn = me:dialog(npc, '여긴 환상의 섬으로 가는 선착장이 아니지. 아직도 길을 모르는 사람이 있더군. 여기서 남서쪽으로 가야 한다네.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_136_COS002::
    btn = me:dialog(npc, '그럼 샛길이 나오는데, 좀 더 걸어가면 또 다른 선착장이 나올 거야. 거기서 배를 탈 수 있다네!', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_136_COS001
    end

    quest:step(3)
end