function NPC_235(me, npc)
    local sel = me:list(npc, '제게 하실 말씀이 있으신가요?', {
        '청자다람쥐인형',
        '잃어버린 장난감'
    })
    if sel == nil then
        return
    end
    if sel == 0 then
        bokgeon_sell_doll(me, npc)
        return
    end
    if sel == 1 then
        bokgeon_find_toys(me, npc)
        return
    end
end

function bokgeon_sell_doll(me, npc)
    local quest = me:quest(QUEST_SELL_DOLL)
    
    if quest == nil then
        if not me:has_items('청자다람쥐인형', 1) then
            me:dialog(npc, '아흠.. 어디 재미있는 장난감이 없나..', false, true)
            return
        end
        local btn = me:dialog(npc, '앗, 귀한 장난감을 가지고 계시네요. 그건... 청자다람쥐인형이군요.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel = me:list(npc, '그걸 제게 파시지 않겠어요?', { '그래, 팔께!', '아니, 됐어.' })
        if sel == nil or sel ~= 0 then
            return
        end
        ::NPC_235_COS001::
        btn = me:dialog(npc, '고맙습니다! 음... 이건 꽤 귀한 장난감이니까 10만전에 살게요!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '다른 재미있는 장난감이 있으면 제게 가져와 주세요. 제가 다 사드릴게요.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_235_COS001
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if not me:start_quest(QUEST_SELL_DOLL) then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        quest = me:quest(QUEST_SELL_DOLL)
        quest:complete()
        me:rmitem('청자다람쥐인형', 1, ITEM_DELETE_TYPE.GIVE)
        me:money(me:money() + 100000)
        return
    end
    
    me:dialog(npc, '다른 재미있는 장난감이 있으면 제게 가져와 주세요. 제가 다 사드릴게요.', false, true)
end

function bokgeon_find_toys(me, npc)
    local quest = me:quest(QUEST_FIND_TOYS)
    
    if quest == nil then
        ::NPC_235_COS002::
        local btn = me:dialog(npc, '아... 대체 어디 있지? 어디서 잃어버렸더라? 뒤적뒤적...', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel = me:list(npc, '어, 누구세요? 제게 하실 말씀이라도 있나요?', { '난 그냥 지나가는 사람인데. 뭘 그리 찾고 있니?', '아니, 아무것도 아니야.' })
        if sel == nil or sel ~= 0 then
            return
        end
        btn = me:dialog(npc, '아, 잃어버린 장난감을 찾고 있어요. 여기 복건성에 계시는 관리가 제 아버님이시죠.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        sel = me:list(npc, '귀한 장난감들인데, 이번에 또 잃어버리면 아버님이 더 이상 사주시지 않겠대요. 이를 어쩌지...', { '장난감은 내가 찾아 줄 테니 여기서 기다리렴.', '그래, 열심히 찾으렴' })
        if sel == nil or sel ~= 0 then
            return
        end
        ::NPC_235_COS003::
        btn = me:dialog(npc, '어, 정말요? 고맙습니다! 찾아주시면 저도 뭔가 꼭 보답을 할게요.\n\n잃어버린 장난감은 상아주사위, 오색폭죽, 청옥팽이예요.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_235_COS003B::
        btn = me:dialog(npc, '상아주사위는 여기 복건성 어딘가에서 잃어버렸고......\n\n오색폭죽은 상해 어딘가에서 잃어버렸고......\n\n청옥팽이는 강서성에서 잃어버렸죠.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_235_COS003
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '그럼 저는 여기서 기다릴께요. 장난감을 찾으시거든 제게 말씀해 주세요.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_235_COS003B
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if not me:start_quest(QUEST_FIND_TOYS) then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        quest = me:quest(QUEST_FIND_TOYS)
        me:push_achievement(37, '복건성태자의 잃어버린 장난감을 찾아주자.', 7, 1)
        return
    end
    
    if quest:completed() then
        me:dialog(npc, '그 때는 정말 감사했습니다.', false, true)
        return
    end
    
    local materials = {['상아주사위'] = 1, ['오색폭죽'] = 1, ['청옥팽이'] = 1}
    if me:has_items(materials) then
        ::NPC_235_COS004::
        local btn = me:dialog(npc, '우와! 상아주사위, 오색폭죽, 청옥팽이를 모두 찾아 주셨군요! 대단해요! 정말 고맙습니다.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '찾느라 힘드셨죠? 이건 팔과탕이라는 요리인데, 맛도 좋고 약으로도 쓰이는 우리 전통요리랍니다. 한 번 드셔보세요.', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_235_COS004
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if me:mkitem('팔과탕', 10) == nil then
            me:dialog(npc, '소지품이 가득 차서 팔과탕을 받을 수 없습니다.', false, true)
            return
        end
        me:rmitem(materials, ITEM_DELETE_TYPE.GIVE)
        quest:complete()
        me:erase_achievement(37)
        return
    else
        me:dialog(npc, '상아주사위는 복건성에서, 오색폭죽은 상해어딘가에서, 청옥팽이는 강서성에서 잃어버렸어요.', false, true)
    end
end