function NPC_105(me, npc)
    local quest = me:quest(QUEST_SAILOR)
    local btn

    if quest ~= nil and quest:step() == 4 then
        ::NPC_105_COS004::
        btn = me:dialog(npc, '또 보는구만. 전에 환상의 섬에서 봤었지? 난 조만간에 환상의 섬으로 또 놀러갈 거라네.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_105_COS005::
        btn = me:dialog(npc, '언제 가냐고? 마음 내키면 언제든지 갈 생각이야! 배표도 싸고 하니 여행하기 딱 좋더구만. 허허허...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_105_COS004
        end
        quest:step(5)
        return
    end

    if quest ~= nil then
        me:dialog(npc, '....', false, false)
        return
    end

    ::NPC_105_COS001::
    btn = me:dialog(npc, '환상의 섬에 가 본적이 있는가? 요즘 다들 한번 가 보자고 난리던데, 다녀온 사람들은 여행하기 딱 좋은 곳이라고 하더군.\n\n특히 전망대에서 내려다보는 풍경이 좋다고 하더구만!', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_105_COS002::
    btn = me:dialog(npc, '그래서 지금 나도 가 보려고 하는 중일세. 자네도 생각 있으면 가 보시게나.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_105_COS001
    end
    ::NPC_105_COS003::
    btn = me:dialog(npc, '길은 아직 나도 확실히는 모르겠고, 일단 성 남문으로 나가면서 사람들에게 물어보려고 하는데... 뭐, 혹시 남문에서 보게 되거든 아는체라도 해 보게나. 허허...', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_105_COS002
    end

    quest = me:start_quest(QUEST_SAILOR)
    if quest == nil then
        return
    end
    quest:step(1)
end