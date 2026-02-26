local ACHIEVEMENT_500 = 13
local REQUIRED_ITEMS = {
    ['도삭산살쾡이가죽'] = 300,
    ['도삭산악어비늘'] = 200,
}
local REWARD_500 = '수정의귀걸이'

function NPC_106(me, npc)
    local quest = me:quest(QUEST_DOTAEYEON)
    if quest == nil then
        return
    end

    if quest:completed() then
        me:dialog(npc, '저번에 날 도와줘서 고마우이. 가죽은 아주 유용하게 사용하고 있다네.', false, true)
        return
    end
    
    local step = quest:step()
    if step == 2 then
        ::NPC_106_1::
        local btn = me:dialog(npc, me:name() .. '씨 아닌가, 400층의 아들이 코뿔소의 뿔등을 얻었다고 굉장히 좋아하더구만.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_106_2::
        btn = me:dialog(npc, '그런데 이 근처에 갑자기 사나운 동물들이 늘어나고 있어. 내 손녀는 자꾸 나가 놀겠다고 조르는데..\n\n얼마전부터 살쾡이와 악어가 늘어나기 시작해서 이제는 아예 집으로 들어오기도 할 정도란 말이지.', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_106_1
        end
        ::NPC_106_3::
        btn = me:dialog(npc, '어른들도 함부로 대들지 못하는터라 다들 집에서 꼼짝못하고 있다우\n\n원래 도삭산 동물들은 온순해서 사람에게 해를 끼치는건 생각도 못했는데 왜 갑자기 이러는 건지..', true, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_106_2
        end
        local sel, list_btn = me:list(npc, '우리를 좀 도와주시게, 코뿔소를 그렇게 쉽게 잡았으니 살쾡이와 악어를 내쫓을 수 있지 않을까?', { '그러죠. 제가 처리하겠습니다!!', '저보다 나은 적임자가 분명 있을겁니다.' }, false)
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == 1 then
            me:dialog(npc, '그런가? 그럼 좀 더 기다려봐야겠군..', false, false)
            return
        end
        if sel ~= 0 then
            return
        end
        quest:step(3)
        me:push_achievement(ACHIEVEMENT_500, '황만령의 부탁을 들어주자!', 7, 1)
        ::NPC_106_5::
        btn = me:dialog(npc, '정말 고맙네.. 살쾡이와 악어는 가죽이 질겨서 쓸곳이 참 많단 말야. 살쾡이와 악어를 처치하고 올때 가죽도 좀 가져다 줄 수 있겠나?', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_106_6::
        btn = me:dialog(npc, '도삭산 살쾡이가죽 300개와 도삭산악어비늘 200개를 구해다주면 내 보물을 드리도록 하지.\n\n살쾡이와 악어는 무리를 지어서 공격해오니 조심하게!', true, true)
        if btn == DIALOG_RESULT.PREV then
            goto NPC_106_5
        end
        return
    end

    if step == 3 then
        if not me:has_items(REQUIRED_ITEMS) then
            me:dialog(npc, '아직 다 구하지 못한 모양이군.. 도삭산 살쾡이가죽 300개와 도삭산악어비늘 200개를 구해다주면 내 보물을 드리도록 하지.', false, false)
            return
        end
        local btn = me:dialog(npc, '오~ 정말 대단하이. 이렇게나 많이. 다친곳은 없나? 그러면 다 가져가겠네.', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if not me:rmitem(REQUIRED_ITEMS, ITEM_DELETE_TYPE.GIVE) then
            me:dialog(npc, '아직 다 구하지 못한 모양이군.. 도삭산 살쾡이가죽 300개와 도삭산악어비늘 200개를 구해다주면 내 보물을 드리도록 하지.', false, false)
            return
        end
        if me:mkitem(REWARD_500, 1) == nil then
            me:dialog(npc, '소지품이 가득 차서 ' .. name_with(REWARD_500, '을', '를') .. ' 받을 수 없습니다.', false, false)
            return
        end
        quest:complete()
        me:push_achievement(ACHIEVEMENT_500, '도삭산 500층 퀘스트 완료', 7, 1)
        btn = me:dialog(npc, '자 여기 내가 주는 선물이라네. 도와줘서 고맙네, 항상 좋은 일만 있게나!', false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    me:dialog(npc, '에구구... 손녀딸이 자꾸 나가 논다는데 위험에서 큰일이야..', false, false)
end