-- npc: 무두
local quest = require('lib.quest')
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
    ::NPC_158_0001::
        local index, button = me:list(npc, '안녕하세요. 어떻게 오셨나요?', {'초혼술방법', '무한 복장', '일반 복장'})
        if index == nil then
            return
        end

        if index == 1 then
            local q = me:quest(quest.QUEST_SHARK_WEAPON)
            if q == nil then
                me:dialog(npc, '자네는 아직 알 때가 아니군.', { prev = false, next = false })
                goto NPC_158_0001
            end
            local step = q:step()
            if step < 3 then
                me:dialog(npc, '자네는 아직 알 때가 아니군.', { prev = false, next = false })
                goto NPC_158_0001
            end
            if step == 3 then
                if me:dialog(npc, '상어장군을 저승에서 불러내고 싶다고?\n\n매우 어려운 일을 부탁하는구만.', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
                    return
                end
                if me:dialog(npc, '사어장군을 소환하기 위해서는 용궁의정기가 필요하다네.', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
                    return
                end
                if me:dialog(npc, '용궁의정기를 만드는 것은 내가 모르니 다른 사람한테 가서 물어보시게.', { prev = false, next = false }) == DIALOG_RESULT.QUIT then
                    return
                end
                if q then
                    q:step(4)
                end
                me:push_achievement(24, '초혼술 방법을 알아보자.', 7, 11)
                goto NPC_158_0001
            end
            if step >= 4 and step <= 9 then
                me:dialog(npc, '아직 용궁의정기를 다 모으지 못했나 보군..', { prev = false, next = false })
                goto NPC_158_0001
            end
            if step == 10 then
                local required_jungki = { ['용궁의정기1'] = 1, ['용궁의정기2'] = 1, ['용궁의정기3'] = 1, ['용궁의정기4'] = 1, ['용궁의정기5'] = 1 }
                if me:dialog(npc, '용궁의정기를 가져왔군.', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
                    return
                end
                if me:dialog(npc, '그럼 어디보자...아수라 마차라 바하라 미다라...하아아압!!!', { prev = true, next = true }) == DIALOG_RESULT.QUIT then
                    return
                end
                local ex_code = me:exchange(
                    { ['item'] = required_jungki },
                    { ['item'] = { ['무기제조법'] = 1 } }
                )
                if ex_code == enum.exchange_result.LACK_COST then
                    me:dialog(npc, '아직 용궁의정기를 다 모으지 못했나 보군..', { prev = false, next = false })
                    goto NPC_158_0001
                end
                if ex_code == enum.exchange_result.LACK_CAPACITY then
                    me:dialog(npc, '소지품이 가득 차서 무기제조법을 받을 수 없네. 자리 좀 비우고 다시 오게.', { prev = false, next = false })
                    goto NPC_158_0001
                end
                if q then
                    q:step(11)
                end
                me:push_achievement(24, '무기제조법을 얻다.', 7, 11)
                me:dialog(npc, '상어장군이 하는 말을 적었으니 이 두루마리를 가져가게.', { prev = false, next = false })
                goto NPC_158_0001
            end
            if step >= 11 then
                me:dialog(npc, '......', { prev = false, next = false })
            end
            goto NPC_158_0001
        end

        if index == 2 then
            local price = 1000
            index = me:list(npc, string.format('무한에 참여하시려면 염색비 %d전을 내시고 무한 복장을 하셔야 합니다. %d전을 내시겠습니까?', price, price), {'예', '아니오'})
            if index == nil then
                return
            end

            if index ~= 1 then
                return
            end

            index = me:list(npc, '어떤 팀으로 참가하시겠습니까?', {'북팀 - 검은색', '남팀 - 붉은색', '동팀 - 파란색', '서팀 - 하얀색'})
            if index == nil then
                return
            end

            if index == 1 then
                me:armor_color(10);
            elseif index == 2 then
                me:armor_color(31);
            elseif index == 3 then
                me:armor_color(17);
            elseif index == 4 then
                me:armor_color(11);
            else
            end

            local money = me:money()
            if money < price then
                me:dialog(npc, '돈이 모자랍니다.')
                return
            end

            me:money(money - price)
    ::NPC_158_0010::
            if me:dialog(npc, string.format('염색비로 %d전을 받았습니다.', price), { prev = false, next = true }) == DIALOG_RESULT.QUIT then
                return
            end

            button = me:dialog(npc, '그럼 언제나 행운이 함께 하시기를 빌겠습니다.', { prev = true, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end

            if button == DIALOG_RESULT.PREV then
                goto NPC_158_0010
            end

            goto NPC_158_0001

        elseif index == 3 then
        	index = me:list(npc, '일반 복장으로 바꿔드릴까요?', {'예', '아니오'})
        	if index == nil then
        		return
        	end

        	if index ~= 1 then
        		return
        	end

        	me:armor_color(nil)
        	button = me:dialog(npc, '일반 복장으로 바꿔드렸습니다.', { prev = false, next = true })
        	if button == DIALOG_RESULT.QUIT then
        		return
        	end

        	goto NPC_158_0001
        else
        end
    end
}
