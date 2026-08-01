-- npc: 나무꾼
local quest = require('lib.quest')
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        if me:level() < 30 then
            me:dialog(npc, '자네는 아직 나무를 하기엔 많이 부족해 보이는군.', { prev = false, next = true })
            return
        end

        local q = me:quest(quest.QUEST_NAMGUN)

        if q == nil then
            ::NPC_180_0001::
            local button = me:dialog(npc, '나무를 하러 오셨소? 이 근처는 나무가 많지는 않지만 좋은 나무가 많아서 나무 하기에 제격이지.', { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end

            ::NPC_180_0002::
            button = me:dialog(npc, '하지만 나무꾼치고는 옷차람이 특이하시구려. 어라, 게다가 도끼도 들고 오지 않은 것 같구려. 잠이 덜 깨셨소? 허허...', { prev = true, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if button == DIALOG_RESULT.PREV then
                goto NPC_180_0001
            end

            local selected, list_button = me:list(npc, '괜찮다면 내 도끼를 주겠소이다. 나는 마침 나무를 다 하고 쉬는 중이라, 필요가 없다오. 집에 도끼는 잔뜩 있고 말이오.', {'예, 빌려주세요.', '고맙지만 사양할께요.'}, { prev = false })
            if list_button == DIALOG_RESULT.QUIT then
                return
            end

            if selected == 1 then
                local q = me:start_quest(quest.QUEST_NAMGUN)
                if q == nil then
                    me:dialog(npc, '퀘스트 시작 실패', { prev = false, next = true })
                    return
                end
                me:push_achievement(28, '쇠도끼로 나무를 하자.', 7, 5)

                if me:mkitem('쇠도끼', 1) == nil then
                    me:dialog(npc, '소지품이 가득 차서 줄 수가 없네.', { prev = false, next = true })
                    return
                end
                me:dialog(npc, '그럼 여기 도끼가 있으니 잘 쓰시오. 그럼 난 낮잠이나 자야겠구려. 나무 열심히 하시오...', { prev = false, next = true })
            end
            return
        end

        if q:completed() then
            me:dialog(npc, 'zzZ...zzZ...', { prev = false, next = true })
            return
        end

        local selected, list_button = me:list(npc, '이미 한 번 쇠도끼를 빌려가지 않았소? 이제 별로 남은 수량이 없어서.. 다시 빌리려면 1만전을 내셔야 하오.', {'다시 빌려주세요.', '필요없어요.'}, { prev = false })
        if list_button == DIALOG_RESULT.QUIT then
            return
        end
        if list_button == DIALOG_RESULT.PREV then
            return
        end
        if selected ~= 1 then
            return
        end

        local code = me:exchange(
            { ['money'] = 10000 },
            { ['item'] = { ['쇠도끼'] = 1 } }
        )
        if code == enum.exchange_result.LACK_COST then
            me:dialog(npc, '돈이 부족한 것은 아니오?', { prev = false, next = true })
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 줄 수가 없네.', { prev = false, next = true })
            return
        end
        me:dialog(npc, '자, 여기있소. 또 잃어버리지 않게 조심하시오.', { prev = false, next = true })
    end
}
