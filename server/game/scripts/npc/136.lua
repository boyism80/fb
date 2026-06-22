-- npc: 선착행자
local quest = require('lib.quest')

function NPC_136(me, npc)
    local q = me:quest(quest.QUEST_SAILOR)
    local btn

    if q ~= nil and q:step() == 6 then
        ::NPC_136_0001::
        btn = me:dialog(npc, '하하, 환상의 섬은 정말 즐거운 곳이었어. 그렇지 않나? 난 지금 그곳에서만 구할 수 있다는 특산품을 사러 가는 중일세.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_136_0010::
        btn = me:dialog(npc, '특히 그곳에서만 구할 수 있다는 요리를 이야기를 들으니 이거 먹고 싶어서 참을 수가 없더구만. 허허...', false, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_136_0001
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        q:step(7)
        return
    end

    if q == nil or q:step() ~= 2 then
        me:dialog(npc, '....', false, false)
        return
    end

    ::NPC_136_0020::
    btn = me:dialog(npc, '여긴 환상의 섬으로 가는 선착장이 아니지. 아직도 길을 모르는 사람이 있더군. 여기서 남서쪽으로 가야 한다네.', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_136_0021::
    btn = me:dialog(npc, '그럼 샛길이 나오는데, 좀 더 걸어가면 또 다른 선착장이 나올 거야. 거기서 배를 탈 수 있다네!', false, true)
    if btn == DIALOG_RESULT.PREV then
        goto NPC_136_0020
    end
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    q:step(3)
end
