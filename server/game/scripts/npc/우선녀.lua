function NPC_77(me, npc)
    local ACHIEVEMENT_CLEAR = 23
    local quest = me:quest(QUEST_CLEAR_SHIELD)
    local btn, sel

    if quest == nil then
        ::NPC_77_COS001::
        btn = me:dialog(npc, '요즘 용궁을 드나드는 외분인들이 부쩍 늘었어요.\n\n그래서 용궁의 물이 점차 더러워 지고 있는 것 같아요.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_77_COS002::
        btn = me:dialog(npc, '용궁의 물을 다시 정화시키는 방법을 알고 싶은데 그 방법을 마침 랑구륜이 알고 있다고 하더라구요.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_77_COS001
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_77_COS003::
        sel = me:list(npc, '가서 좀 알아봐 주실 수 있나요?', { '물론입니다.', '별로..내키지가 않아서...' })
        if sel == nil then
            return
        end
        if sel == 1 then
            me:dialog(npc, '별로..내키지가 않아서...', false, true)
            return
        end
        if sel ~= 0 then
            return
        end
        if not me:start_quest(QUEST_CLEAR_SHIELD) then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        quest = me:quest(QUEST_CLEAR_SHIELD)
        quest:step(1)
        me:push_achievement(ACHIEVEMENT_CLEAR, '용궁정화 퀘스트를 받다.', 7, 1)
        me:dialog(npc, '고마워요.\n\n한시라도 빨리 정화하는 방법을 알아다 주세요.', false, true)
        return
    end

    if quest:completed() then
        me:dialog(npc, me:name() .. '덕분에 용궁의 물이 깨끗해졌어요.', false, true)
        return
    end

    if quest:step() == 1 or quest:step() == 2 then
        me:dialog(npc, '물을 정화시키는 방법은 랑구륜에게 가보시면 될거에요.', false, true)
        return
    end

    if quest:step() == 3 then
        if not me:has_items('정화비서', 1) then
            me:dialog(npc, '아직 물을 정화시키는 법을 알아오지 못하셨군요.', false, false)
            return
        end
        ::NPC_77_COS004::
        btn = me:dialog(npc, '아 비법을 알아오셨네요.\n\n어디 한번 볼까요.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_77_COS005::
        btn = me:dialog(npc, '음........으으으으음..........\n\n아. 이렇게 간단한 것을....', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_77_COS004
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_77_COS006::
        btn = me:dialog(npc, '하지만 이 방법을 실행하기 위해서는 숯의정화가 필요하네요. 가서 숯의정화 3조각만 가져다 주세요.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_77_COS005
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_77_COS007::
        btn = me:dialog(npc, '숯의정화를 만들기 위해서는 불의 힘이 깃든 물건이 필요하겠네요.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_77_COS006
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        quest:step(4)
        me:rmitem('정화비서', 1, ITEM_DELETE_TYPE.GIVE)
        me:push_achievement(ACHIEVEMENT_CLEAR, '우선녀의 부탁을 들어주자.', 7, 1)
        return
    end

    if quest:step() == 4 then
        local materials = {['숯의정화'] = 3}
        if not me:has_items(materials) then
            me:dialog(npc, '물을 정화하기 위해 숯의정화 3조각만 구해주세요.', false, false)
            return
        end
        if me:mkitem('정화의방패', 1) == nil then
            me:dialog(npc, '소지품이 가득 차서 정화의방패를 받을 수 없습니다.', false, true)
            return
        end
        me:rmitem(materials, ITEM_DELETE_TYPE.GIVE)
        quest:step(5)
        quest:complete()
        me:push_achievement(ACHIEVEMENT_CLEAR, '용궁정화에 성공하였다.', 7, 1)
        ::NPC_77_COS008::
        btn = me:dialog(npc, '아. 이제야 용궁이 다시 깨끗함을 찾을 수 있겠네요.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_77_COS009::
        btn = me:dialog(npc, '이건 감사의 뜻으로 드리는 것이니까 받아두세요.', false, false)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_77_COS008
        end
        return
    end
end