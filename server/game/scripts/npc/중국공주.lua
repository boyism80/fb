function NPC_143(me, npc)
    local quest = me:quest(QUEST_PRINCESS_RING)

    if quest == nil then
        ::NPC_143_COS001::
        local btn = me:dialog(npc, '제가 강에서 배를 타다가 반지를 빠뜨리고 말았어요. 그 반지는 옆나라의 제 정혼자인 왕자가 준 반지라 잃어버리면 그 나라와의 관계까지 위험해질수 있어요.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_143_COS002::
        btn = me:dialog(npc, '찾으러 여러분이 들어갔지만 아직도 나오신 분이 없네요. 아무래도 평범한 강이 아닌거 같아요.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_143_COS001
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_143_COS003::
        local sel = me:list(npc, '제발 부탁드려요. 반지를 찾아주세요.', { '걱정마세요. 제가 꼭 찾아드리죠.', '흠. 전 물을 싫어해서요. 죄송하네요.' })
        if sel == nil or sel == 1 then
            if sel == 1 then
                me:dialog(npc, '그러시다면야.. 이를 어쩐담...', false, true)
            end
            return
        end
        btn = me:dialog(npc, '정말 고마워요. 그럼 믿고 기다리죠.', false, true)
        if not me:start_quest(QUEST_PRINCESS_RING) then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        me:push_achievement(3, '중국공주의 잃어버린 반지를 되찾자.', 7, 1)
        return
    end

    if quest:completed() then
        me:dialog(npc, '저번엔 정말로 감사했습니다.', false, true)
        return
    end

    -- in progress: need 공주의반지
    if not me:has_items('공주의반지', 1) then
        me:dialog(npc, '아직 제 반지가 없으신거같은데..', false, true)
        return
    end
    local btn = me:dialog(npc, '정말 감사해요. 정말... 이 은혜는 잊지 못할꺼에요. 이건 약소하지만 제가 드리는 선물입니다.', false, true)
    if me:mkitem('청옥반지', 1) == nil then
        me:dialog(npc, '소지품이 가득 차서 청옥반지를 받을 수 없습니다.', false, true)
        return
    end
    me:rmitem('공주의반지', 1, ITEM_DELETE_TYPE.GIVE)
    quest:complete()
    me:push_achievement(3, '잃어버린 공주의 반지를 되찾아 주다.', 7, 1)
end