function NPC_233(me, npc)
    if me:level() < 56 then
        me:dialog(npc, '(옆에 있는 아이를 보며 크게 한숨을 내쉰다.)', false, true)
        return
    end

    local quest = me:quest(QUEST_SICK_CHILD)

    if quest == nil then
        local sel, btn = me:list(npc, '아... 가난이 원수지, 병든 네게 보약 한 첩 지어줄 형편도 못 되는구나! 이럴 때 네 아버지가 있었다면... 에휴...', { '저... 무슨 일이라도 있나요?', '아픈 사람이라도 있나? 뭐, 나랑은 상관없지...' }, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel ~= 0 then
            return
        end
        ::NPC_233_COS001::
        btn = me:dialog(npc, '아... 들으셨나요? .....제 옆에 있는 아이는 저희 아들인데, 얼마 전에 병을 앓고 나더니 기력이 쇠했는지 기운이 없고 말도 못해요.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_233_COS002::
        btn = me:dialog(npc, '아이 아빠는 오래 전에 돈을 벌겠다고 ㅇ ㅓ디론가 떠나서는 소식이 없고, 저 혼자 남의 집 삯바느질로 먹고 사는데, 이 애에게 보약 한 첩 지어 먹일 형편도 못 되네요.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_233_COS001
        end
        sel, btn = me:list(npc, '이대로 가다간 혹시 이 애가 아예 말을 못하게 되는 건 아닐까 겁이 덜컥 나서, 저도 모르게 그만 소리를 내어 한탄하고 말았군요...', { '그런 가엾은 일이... 제가 도와드리겠습니다!', '그래요? 뭐, 저랑은 상관없는 일이니까 전 이만..' }, true)
        if btn == DIALOG_RESULT.QUIT or sel ~= 0 then
            return
        end
        if not me:start_quest(QUEST_SICK_CHILD) then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        quest = me:quest(QUEST_SICK_CHILD)
        quest:step(1)
        me:push_achievement(36, '아픈 아이를 위해 보약을 구하자.', 7, 1)
        ::NPC_233_COS003::
        btn = me:dialog(npc, '예? 정말이세요? 이렇게 고마울 데가...\n\n아시다시피 보약은 워낙 비싸서, 저희 형편으로는 지을 수가 없답니다.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_233_COS004::
        btn = me:dialog(npc, '그런데... 재료만 가져다 주면 어떤 보약이든 공짜로 지어주는 사람이 장안성 푸줏간에 있다고 해요.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_233_COS003
        end
        ::NPC_233_COS005::
        me:dialog(npc, '그 사람이라면 저희 아들의 증세만 듣고도 보약을 지어줄 것 같은데... 정말 죄송하지만 한번 그분께 여쭤 봐 주세요. 부탁드립니다...', true, true)
        return
    end

    if quest:step() == 1 then
        if not me:has_items('삼전신보탕', 1) then
            me:dialog(npc, '보약은 장안성 푸줏간에 가보시면 될거예요.', false, true)
            return
        end
        local sel, btn = me:list(npc, '아아...! 보약을 정말로 구해다 주셨군요! 이 은혜를 어찌 갚아야 할지...', { '사례는 필요없습니다. 자, 이 약을 받으세요.', '아무리 가난해도... 수고비 정도는 주시겠죠?' }, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == 0 then
            if me:mkitem('청자다람쥐인형', 1) == nil then
                me:dialog(npc, '소지품이 가득 차서 청자다람쥐인형을 줄 수 없습니다.', false, true)
                return
            end
            quest:step(2)
            me:rmitem('삼전신보탕', 1, ITEM_DELETE_TYPE.GIVE)
            me:push_achievement(36, '아픈 아이의 병을 치료해 주었다.', 7, 1)
            ::NPC_233_COS006::
            btn = me:dialog(npc, '감사합니다... 정말 감사합니다... 이 은혜 언젠가는 꼭 갚겠어요.......', true, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_233_COS007::
            btn = me:dialog(npc, '......아, 이건 저희 애가 가지고 놀던 장남간인데, 보잘것 없어 보여도 아이 아빠가 예전에 귀한 장난감이라면서 가져온 것이랍니다.', true, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_233_COS006
            end
            me:dialog(npc, '오래된 장난감이니 누군가 장난감을 수집하는 사람이라도 있다면, 비싸게 팔 수 있을 거예요.', true, true)
        elseif sel == 1 then
            me:dialog(npc, '저... 그.. 그게...', false, true)
        end
        return
    end

    if quest:step() == 0 then
        local sel, btn = me:list(npc, '아... 가난이 원수지, 병든 네게 보약 한 첩 지어줄 형편도 못 되는구나! 이럴 때 네 아버지가 있었다면... 에휴...', { '저... 무슨 일이라도 있나요?', '아픈 사람이라도 있나? 뭐, 나랑은 상관없지...' }, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel ~= 0 then
            return
        end
        ::NPC_233_COS001B::
        btn = me:dialog(npc, '아... 들으셨나요? .....제 옆에 있는 아이는 저희 아들인데, 얼마 전에 병을 앓고 나더니 기력이 쇠했는지 기운이 없고 말도 못해요.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_233_COS002B::
        btn = me:dialog(npc, '아이 아빠는 오래 전에 돈을 벌겠다고 ㅇ ㅓ디론가 떠나서는 소식이 없고, 저 혼자 남의 집 삯바느질로 먹고 사는데, 이 애에게 보약 한 첩 지어 먹일 형편도 못 되네요.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_233_COS001B
        end
        sel, btn = me:list(npc, '이대로 가다간 혹시 이 애가 아예 말을 못하게 되는 건 아닐까 겁이 덜컥 나서, 저도 모르게 그만 소리를 내어 한탄하고 말았군요...', { '그런 가엾은 일이... 제가 도와드리겠습니다!', '그래요? 뭐, 저랑은 상관없는 일이니까 전 이만..' }, true)
        if btn == DIALOG_RESULT.QUIT or sel ~= 0 then
            return
        end
        quest:step(1)
        me:push_achievement(36, '아픈 아이를 위해 보약을 구하자.', 7, 1)
        ::NPC_233_COS003B::
        btn = me:dialog(npc, '예? 정말이세요? 이렇게 고마울 데가...\n\n아시다시피 보약은 워낙 비싸서, 저희 형편으로는 지을 수가 없답니다.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_233_COS004B::
        btn = me:dialog(npc, '그런데... 재료만 가져다 주면 어떤 보약이든 공짜로 지어주는 사람이 장안성 푸줏간에 있다고 해요.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_233_COS003B
        end
        ::NPC_233_COS005B::
        me:dialog(npc, '그 사람이라면 저희 아들의 증세만 듣고도 보약을 지어줄 것 같은데... 정말 죄송하지만 한번 그분께 여쭤 봐 주세요. 부탁드립니다...', true, true)
        return
    end

    me:dialog(npc, '저번엔 정말 감사했습니다. 아이가 덕분에 건강을 많이 회복했습니다.', true, true)
end