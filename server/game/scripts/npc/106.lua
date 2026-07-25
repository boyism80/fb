-- npc: 황만령
local quest = require('lib.quest')
local enum = require('lib.enum')

local ACHIEVEMENT_500 = 13
local REQUIRED_ITEMS = {
    ['도삭산살쾡이가죽'] = 300,
    ['도삭산악어비늘'] = 200,
}
local REWARD_500 = '수정의귀걸이'

return {
    ON_CLICK = function(me, npc)
        local q = me:quest(quest.QUEST_DOTAEYEON)
        if q == nil then
            return
        end

        if q:completed() then
            me:dialog(npc, '저번에 날 도와줘서 고마우이. 가죽은 아주 유용하게 사용하고 있다네.', { prev = false, next = true })
            return
        end

        local step = q:step()
        if step == 2 then
            ::NPC_106_0001::
            local btn = me:dialog(npc, me:name() .. '씨 아닌가, 400층의 아들이 코뿔소의 뿔등을 얻었다고 굉장히 좋아하더구만.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_106_0002::
            btn = me:dialog(npc, '그런데 이 근처에 갑자기 사나운 동물들이 늘어나고 있어. 내 손녀는 자꾸 나가 놀겠다고 조르는데..\n\n얼마전부터 살쾡이와 악어가 늘어나기 시작해서 이제는 아예 집으로 들어오기도 할 정도란 말이지.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_106_0001
            end
            ::NPC_106_0003::
            btn = me:dialog(npc, '어른들도 함부로 대들지 못하는터라 다들 집에서 꼼짝못하고 있다우\n\n원래 도삭산 동물들은 온순해서 사람에게 해를 끼치는건 생각도 못했는데 왜 갑자기 이러는 건지..', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_106_0002
            end
            local sel, list_btn = me:list(npc, '우리를 좀 도와주시게, 코뿔소를 그렇게 쉽게 잡았으니 살쾡이와 악어를 내쫓을 수 있지 않을까?', { '그러죠. 제가 처리하겠습니다!!', '저보다 나은 적임자가 분명 있을겁니다.' }, { prev = false })
            if list_btn == DIALOG_RESULT.QUIT then
                return
            end
            if sel == 2 then
                me:dialog(npc, '그런가? 그럼 좀 더 기다려봐야겠군..', { prev = false, next = false })
                return
            end
            if sel ~= 1 then
                return
            end
            q:step(3)
            me:push_achievement(ACHIEVEMENT_500, '황만령의 부탁을 들어주자!', 7, 1)
            ::NPC_106_0005::
            btn = me:dialog(npc, '정말 고맙네.. 살쾡이와 악어는 가죽이 질겨서 쓸곳이 참 많단 말야. 살쾡이와 악어를 처치하고 올때 가죽도 좀 가져다 줄 수 있겠나?', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_106_0006::
            btn = me:dialog(npc, '도삭산 살쾡이가죽 300개와 도삭산악어비늘 200개를 구해다주면 내 보물을 드리도록 하지.\n\n살쾡이와 악어는 무리를 지어서 공격해오니 조심하게!', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_106_0005
            end
            return
        end

        if step == 3 then
            if not me:has_items(REQUIRED_ITEMS) then
                me:dialog(npc, '아직 다 구하지 못한 모양이군.. 도삭산 살쾡이가죽 300개와 도삭산악어비늘 200개를 구해다주면 내 보물을 드리도록 하지.', { prev = false, next = false })
                return
            end
            local btn = me:dialog(npc, '오~ 정말 대단하이. 이렇게나 많이. 다친곳은 없나? 그러면 다 가져가겠네.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local code = me:exchange(
                { ['item'] = REQUIRED_ITEMS },
                { ['item'] = { [REWARD_500] = 1 } }
            )
            if code == enum.EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, '아직 다 구하지 못한 모양이군.. 도삭산 살쾡이가죽 300개와 도삭산악어비늘 200개를 구해다주면 내 보물을 드리도록 하지.', { prev = false, next = false })
                return
            end
            if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, '소지품이 가득 차서 ' .. name_with(REWARD_500, '을', '를') .. ' 받을 수 없습니다.', { prev = false, next = false })
                return
            end
            q:complete()
            me:push_achievement(ACHIEVEMENT_500, '도삭산 500층 퀘스트 완료', 7, 1)
            btn = me:dialog(npc, '자 여기 내가 주는 선물이라네. 도와줘서 고맙네, 항상 좋은 일만 있게나!', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            return
        end

        me:dialog(npc, '에구구... 손녀딸이 자꾸 나가 논다는데 위험에서 큰일이야..', { prev = false, next = false })
    end
}
