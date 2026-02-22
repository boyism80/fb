local ACHIEVEMENT_RABBIT_CATCH = 20
local ITEM_NEURAEJINDO = '뇌진도'

function NPC_159(me, npc)
    local btn
    local quest = me:quest(QUEST_RABBIT_LIVER)
    if quest == nil then
        me:dialog(npc, '지금은 이야기를 할 때가 아니군..', false, false)
        return
    end

    local sel = me:list(npc, '어쩐일이신가?', { '토끼의간을 구해야되는데..', '뇌진도를 드리겠습니다.' }, false)
    if sel == nil then
        return
    end

    if sel == 0 then
        ::NPC_159_COS001::
        local btn = me:dialog(npc, '음.. 거북장군에게서 토끼의간을 구해오라는 부탁을 받았단 말아지. 그런데 그 토끼는 정말 희귀한 토끼라네. 보통토끼와는 다르지', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_159_COS002::
        btn = me:dialog(npc, '나도 장사 하느라 전국 방방곡곡 안가보는데가 없지만, 그 토끼는 딱 한 번 밖에 못봤었지.. 어디서 보았냐 하면 말일세...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_159_COS001
        end
        ::NPC_159_COS003::
        btn = me:dialog(npc, '흐흐흐.. 이보게, 하지만 말일세.. 조금 야속하게 들릴지 모르겠지만, 나도 장사꾼이 아닌가?', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_159_COS002
        end
        ::NPC_159_COS004::
        local choice, list_btn = me:list(npc, '흐흐흐...', { '원하는게 뭡니까' }, false)
        if choice == nil or choice ~= 0 then
            return
        end
        me:dialog(npc, '요즘 한창 장사가 되는 물건이 뇌진도라네.. 그걸 하나 구해오면 내 분명히 자네에게 그 토끼가 출현하는 장소를 가르쳐 줌세.', false, false)
        return
    end
    if not me:has_items(ITEM_NEURAEJINDO, 1) then
        me:dialog(npc, '뇌진도가 없지 않은가?', false, false)
        return
    end
    quest:step(2)
    me:rmitem(ITEM_NEURAEJINDO, 1, ITEM_DELETE_TYPE.GIVE)
    me:push_achievement(ACHIEVEMENT_RABBIT_CATCH, '토깽이를 생포하자.', 7, 1)
    ::NPC_159_COS005::
    btn = me:dialog(npc, '그 토끼가 출현하는 곳은 다름 아닌 한두고개라네.. 나도 한두고개에서 깊숙히 안들어가봐서 깊숙한 곳에서도 출현할지 어떨지는 모르겠네만', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::NPC_159_COS006::
    btn = me:dialog(npc, '분명한 것은 한두고개에서 깊지 않은 곳에서 봤다는 사실일세.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto NPC_159_COS005
    end
    ::NPC_159_COS007::
    btn = me:dialog(npc, '그럼 행운을 비네.. 아참, 이건 자네를 위해서 하는 얘기네만, 그 토끼는 아주 간악하다는 사실을 꼭 염두에 두게', true, false)
    if btn == DIALOG_RESULT.PREV then
        goto NPC_159_COS006
    end
end