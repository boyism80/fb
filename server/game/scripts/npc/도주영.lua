function NPC_110(me, npc)
    local quest = me:quest(QUEST_DOJAEYOUNG_HERB)
    if quest == nil then
        me:dialog(npc, '더 예뻐질 방법이 없을까?', false, true)
        return
    end

    if quest:step() < 5 then
        me:dialog(npc, '더 예뻐질 방법이 없을까?', false, true)
        return
    end

    if quest:step() == 5 then
        ::NPC_110_COS001::
        local btn = me:dialog(npc, '앗! ' .. me:name() .. '님 이시군요! 저희 아버지께서 저에게 인어의거울을 주셨는데, 잘 받았습니다.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_110_COS002::
        btn = me:dialog(npc, '이곳 도삭산은 외로운 곳이랍니다. 사람들도 그리 많이 오가지 않고.. 그래서 저는 400층까지의 길을 만리향 꽃으로 가득찬 길로 만들겠다는 생각을 하게 되었죠.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_110_COS001
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_110_COS003::
        btn = me:dialog(npc, '하지만 얼마전에 정성들여 모으던 만리향 씨앗들을 해파리 괴물들에게 빼앗기고 말았어요.\n\n그래서 말인데 제 부탁을 좀 들어주세요. 해파리 괴물들에게 제 만리향 씨앗을 빼앗아 400층까지 가는길 곳곳에 심어 주시면 됩니다.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_110_COS002
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_110_COS004::
        btn = me:dialog(npc, '걸어다니시다 보면 씨앗을 심기에 좋은 장소가 보이실꺼에요.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_110_COS003
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel = me:list(npc, '제 부탁 들어주실거죠??', { '그런 부탁이라면 기꺼이~!', '죄송하지만 저에겐 그럴만한 시간이 없습니다.' })
        if sel == nil then
            return
        end
        if sel ~= 0 then
            return
        end
        quest:step(6)
        me:push_achievement(9, '도주영의 부탁을 들어주자!', 7, 1)
        me:dialog(npc, '씨앗은 해파리 괴물들을 처치하면 얻을 수 있을거에요. 부디 제 부탁을 들어주시길. 그럼 몸조심하세요....', true, true)
        return
    end

    if quest:step() == 6 then
        local progress = quest:progress()
        if progress >= 1 and progress <= 9 then
            me:dialog(npc, '정말 감사해요. 덕분에 도삭산이 많이 보기 좋아졌어요.', true, true)
            return
        end
        if progress >= 1000 then
            if me:mkitem('적막의목걸이', 1) == nil then
                me:dialog(npc, '소지품이 가득 차서 적막의목걸이를 받을 수 없어요.', false, true)
                return
            end
            quest:step(7)
            me:push_achievement(9, '도삭산 300층 퀘스트 완료', 7, 1)
            me:dialog(npc, '흑..정말 감사드려요. 대신 제가 아끼는 물건을 하나 드리죠. 자..', true, true)
            return
        end
        me:dialog(npc, '아직 많이 심지 못하셨군요..', false, true)
        return
    end

    if quest:step() >= 7 then
        me:dialog(npc, '저번엔 정말 감사했어요. 덕분에 도삭산이 많이 보기가 좋아졌어요.', true, true)
    end
end