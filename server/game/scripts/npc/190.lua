-- npc: 하태현
local quest = require('lib.quest')
local enum = require('lib.enum')

local ACHIEVEMENT_700 = 15
local REQUIRED_ITEM = { ['나무가면'] = 50 }
local REWARD_ITEMS = { '진비투구', '경비투구', '법비모', '격비모' }
local REWARD_CLASSES = { '전사', '도적', '주술사', '도사' }

return {
    ON_CLICK = function(me, npc)
        local q = me:quest(quest.QUEST_HATAEHYUN)
        local btn

        if q == nil then
            ::NPC_190_0001::
            btn = me:dialog(npc, '이곳까지 오시느라 수고하셨네요. 끝까지 올라가실 생각인가요? 음.. 계속 올라가실거라면 부탁하나 드리고 싶은데..', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_190_0002::
            btn = me:dialog(npc, '조금더 올라가시다보면 도삭산선비족, 도삭산흉노족이 있답니다. 그녀석들은 포악하고 성격은 나쁘지만 여러가지 신기한것들을 가지고 있죠.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_190_0001
            end
            ::NPC_190_0003::
            btn = me:dialog(npc, '그중에 가면종류가 신기한것이 많죠. 각종 나무가면과 또 철이나 다른 금속으로 만든 가면도 있다더군요.\n\n제가 그런 물건에 관심이 많아서 부탁드립니다만 나무가면 50개를 모아다 주실수 없으신가요? 50개면 종류별로 다 모을 수 있을것 같아서..', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_190_0002
            end
            local sel, list_btn = me:list(npc, '부탁드려요. 워낙 포악해서 저는 근처도 갈 용기가 없네요. 들어주실거죠?', { '그런일이라면 맡겨만 주세요.', '가면이라. 전 그런것에 관심이 없어서요.' }, { prev = false })
            if list_btn == DIALOG_RESULT.QUIT then
                return
            end
            if sel == 2 then
                me:dialog(npc, '그러시군요.. 안타깝네요..', { prev = false, next = false })
                return
            end
            if sel ~= 1 then
                return
            end
            q = me:start_quest(quest.QUEST_HATAEHYUN)
            if q == nil then
                me:dialog(npc, '퀘스트 시작 실패', { prev = false, next = true })
                return
            end
            q:step(1)
            me:push_achievement(ACHIEVEMENT_700, '하태현의 부탁을 들어주자!', 7, 1)
            me:dialog(npc, '그럼 전 ' .. me:name() .. '님만 기다리고 있을께요. 적어도 50개는 모아다 주셔야해요~ 그래야 종류별로. 부탁드릴께요.', { prev = false, next = true })
            return
        end

        if q:completed() then
            me:dialog(npc, '저번엔 정말 감사했습니다. 가면이 하나같이 모두 흥미롭군요.', { prev = false, next = true })
            return
        end

        local step = q:step()
        if step == 1 then
            if not me:has_items(REQUIRED_ITEM) then
                me:dialog(npc, '아직 나무가면이 별로 없네요.. 50개정도만 구해주세요~', { prev = false, next = false })
                return
            end
            ::NPC_190_0005::
            btn = me:dialog(npc, '우와. 역시 정말 멋진 가면들. 정말 감사해요. 제가 가져가도록 하죠.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_190_0006::
            btn = me:dialog(npc, '고마움에 표시로 드리는 선물입니다. 투구인데요 님에게 맞는 투구를 고르세요..', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_190_0005
            end
            local sel2, list_btn2 = me:list(npc, '어떤 투구를 드릴까요?', { '진비투구를 주십시오.', '경비투구를 주십시오.', '법비모를 주십시오.', '격비모를 주십시오.', '어떤 투구들이 있는지 알려주십시오.' }, { prev = false })
            if list_btn2 == DIALOG_RESULT.QUIT then
                return
            end
            if sel2 >= 0 and sel2 <= 4 then
                local item_name = REWARD_ITEMS[sel2]
                local class_name = REWARD_CLASSES[sel2]
                local code = me:exchange(
                    { ['item'] = REQUIRED_ITEM },
                    { ['item'] = { [item_name] = 1 } }
                )
                if code == enum.EXCHANGE_RESULT.LACK_COST then
                    me:dialog(npc, '아직 나무가면이 별로 없네요.. 50개정도만 구해주세요~', { prev = false, next = false })
                    return
                end
                if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                    me:dialog(npc, '소지품이 가득 차서 ' .. name_with(item_name, '을', '를') .. ' 받을 수 없습니다.', { prev = false, next = false })
                    return
                end
                q:complete()
                me:push_achievement(ACHIEVEMENT_700, '도삭산 700층 퀘스트 완료', 7, 1)
                btn = me:dialog(npc, item_name .. '는 ' .. class_name .. '분들을 위한 투구입니다.', { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                btn = me:dialog(npc, '혹시 아실지 모르겠지만 도삭산선비족,도삭산흉노족의 가면을 이용하면 더 좋은 투구를 만드실 수 있을거에요.\n\n저도 자세한 방법은 모르지만요. 하여튼 너무나 감사해요. 다음에 꼭 다시 들러주세요~', { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                return
            end
            if sel2 == 5 then
                ::NPC_190_0008::
                btn = me:dialog(name2item('진비투구'), '진비투구는 전사를 위한 투구입니다.', { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                ::NPC_190_0009::
                btn = me:dialog(name2item('경비투구'), '경비투구는 도적을 위한 투구입니다.', { prev = true, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                if btn == DIALOG_RESULT.PREV then
                    goto NPC_190_0008
                end
                ::NPC_190_0010::
                btn = me:dialog(name2item('법비모'), '법비모는 주술사를 위한 투구입니다.', { prev = true, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                if btn == DIALOG_RESULT.PREV then
                    goto NPC_190_0009
                end
                ::NPC_190_0011::
                btn = me:dialog(name2item('격비모'), '격비모는 도사를 위한 투구입니다.', { prev = false, next = false })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                if btn == DIALOG_RESULT.PREV then
                    goto NPC_190_0010
                end
                return
            end
            return
        end

        me:dialog(npc, '저번엔 정말 감사했습니다. 가면이 하나같이 모두 흥미롭군요.', { prev = false, next = true })
    end
}
