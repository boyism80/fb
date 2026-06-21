local quest = require('lib.quest')
local server = require('lib.server')

function NPC_235(me, npc)
    local q = me:quest(quest.QUEST_PAMASPIRI)
    if q and q:step() == 2 and not q:completed() then
        ::NPC_235_0001::
        local sel, btn = me:list(npc, "누구세요? 제게 무슨 하실 말씀이라도...?", { "혹시 사탕 좀 가지고 있니?", "아무 일도 아니란다." }, false)
        if btn == DIALOG_RESULT.QUIT or sel == nil or sel == 1 then
            return
        end

        ::NPC_235_0002::
        local button = me:dialog(npc, "아, 사탕이 필요하세요? 예, 나눠 드릴께요.", true, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_235_0001
        end

        if me:mkitem("꿀사탕", 1) == nil then
            me:dialog(npc, "소지품이 가득 차서 줄 수 없네요.", false, false)
            return
        end
        q:step(3)
        me:push_achievement(48, "파마의 피리를 찾자(꼬마에게 사탕을 주자).", 7, 1)
        me:dialog(npc, "대신 다음에 시간이 되시면 제 장난감 찾는 일을 좀 도와주셔야 돼요. 아셨죠? 그럼 전 이만...", false, false)
        return
    end

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
    local q = me:quest(quest.QUEST_SELL_DOLL)
    
    if q == nil then
        if not me:has_items('청자다람쥐인형', 1) then
            me:dialog(npc, '아흠.. 어디 재미있는 장난감이 없나..', false, true)
            return
        end
        ::NPC_235_0008::
        local btn = me:dialog(npc, '앗, 귀한 장난감을 가지고 계시네요. 그건... 청자다람쥐인형이군요.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_235_0009::
        local sel, btn = me:list(npc, '그걸 제게 파시지 않겠어요?', { '그래, 팔께!', '아니, 됐어.' }, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_235_0008
        end
        if sel == nil or sel ~= 0 then
            return
        end
        ::NPC_235_0010::
        btn = me:dialog(npc, '고맙습니다! 음... 이건 꽤 귀한 장난감이니까 10만전에 살게요!', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_235_0009
        end
        btn = me:dialog(npc, '다른 재미있는 장난감이 있으면 제게 가져와 주세요. 제가 다 사드릴게요.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        q = me:start_quest(quest.QUEST_SELL_DOLL)
        if q == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        local code = me:exchange(
            { ['item'] = { ['청자다람쥐인형'] = 1 } },
            { ['money'] = 100000 }
        )
        if code == server.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '청자다람쥐인형을 가져오세요.', false, true)
            return
        end
        if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '금전을 받을 여유가 없군요.', false, true)
            return
        end
        q:complete()
        return
    end
    
    me:dialog(npc, '다른 재미있는 장난감이 있으면 제게 가져와 주세요. 제가 다 사드릴게요.', false, true)
end

function bokgeon_find_toys(me, npc)
    local q = me:quest(quest.QUEST_FIND_TOYS)
    
    if q == nil then
        ::NPC_235_0020::
        local btn = me:dialog(npc, '아... 대체 어디 있지? 어디서 잃어버렸더라? 뒤적뒤적...', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local sel, btn = me:list(npc, '어, 누구세요? 제게 하실 말씀이라도 있나요?', { '난 그냥 지나가는 사람인데. 뭘 그리 찾고 있니?', '아니, 아무것도 아니야.' }, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_235_0020
        end
        if sel == nil or sel ~= 0 then
            return
        end
        ::NPC_235_0018::
        btn = me:dialog(npc, '아, 잃어버린 장난감을 찾고 있어요. 여기 복건성에 계시는 관리가 제 아버님이시죠.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_235_0019::
        sel, btn = me:list(npc, '귀한 장난감들인데, 이번에 또 잃어버리면 아버님이 더 이상 사주시지 않겠대요. 이를 어쩌지...', { '장난감은 내가 찾아 줄 테니 여기서 기다리렴.', '그래, 열심히 찾으렴' }, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_235_0018
        end
        if sel == nil or sel ~= 0 then
            return
        end
        ::NPC_235_0021::
        btn = me:dialog(npc, '어, 정말요? 고맙습니다! 찾아주시면 저도 뭔가 꼭 보답을 할게요.\n\n잃어버린 장난감은 상아주사위, 오색폭죽, 청옥팽이예요.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_235_0019
        end
        ::NPC_235_0022::
        btn = me:dialog(npc, '상아주사위는 여기 복건성 어딘가에서 잃어버렸고......\n\n오색폭죽은 상해 어딘가에서 잃어버렸고......\n\n청옥팽이는 강서성에서 잃어버렸죠.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_235_0021
        end
        btn = me:dialog(npc, '그럼 저는 여기서 기다릴께요. 장난감을 찾으시거든 제게 말씀해 주세요.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        q = me:start_quest(quest.QUEST_FIND_TOYS)
        if q == nil then
            me:dialog(npc, '퀘스트 시작 실패', false, true)
            return
        end
        me:push_achievement(37, '복건성태자의 잃어버린 장난감을 찾아주자.', 7, 1)
        return
    end
    
    if q:completed() then
        me:dialog(npc, '그 때는 정말 감사했습니다.', false, true)
        return
    end
    
    local materials = {['상아주사위'] = 1, ['오색폭죽'] = 1, ['청옥팽이'] = 1}
    if me:has_items(materials) then
        ::NPC_235_0030::
        local btn = me:dialog(npc, '우와! 상아주사위, 오색폭죽, 청옥팽이를 모두 찾아 주셨군요! 대단해요! 정말 고맙습니다.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, '찾느라 힘드셨죠? 이건 팔과탕이라는 요리인데, 맛도 좋고 약으로도 쓰이는 우리 전통요리랍니다. 한 번 드셔보세요.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local code = me:exchange(
            { ['item'] = materials },
            { ['item'] = { ['팔과탕'] = 10 } }
        )
        if code == server.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '아직 재료를 다 모으지 못하셨군요.', false, true)
            return
        end
        if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 팔과탕을 받을 수 없습니다.', false, true)
            return
        end
        q:complete()
        me:erase_achievement(37)
        return
    else
        me:dialog(npc, '상아주사위는 복건성에서, 오색폭죽은 상해어딘가에서, 청옥팽이는 강서성에서 잃어버렸어요.', false, true)
    end
end