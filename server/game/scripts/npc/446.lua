-- npc: 사도
local quest = require('lib.quest')
local function parse_hunt_param(param)
    if not param or param == "" then
        return 0, 0
    end
    local a, b = param:match("^(%d+),(%d+)$")
    if a and b then
        return tonumber(a) or 0, tonumber(b) or 0
    end
    return 0, 0
end

return {
    on_click = function(me, npc)
        local q = me:quest(quest.QUEST_BEGINNER_PATH)
        if q == nil then
            me:dialog(npc, "먼저 앞의 도우미 분들의 도움을 받고 오세요.", { prev = false, next = false })
            return
        end

        if q:completed() then
            me:dialog(npc, "이제 다음 방으로 이동하시면 됩니다. 제 윗쪽에 다음 건물의 입구가 있습니다.", { prev = false, next = true })
            me:dialog(npc, "그럼 안녕히가세요 ~~!", { prev = false, next = false })
            return
        end

        local step = q:step()
        if step < 3 then
            me:dialog(npc, "먼저 앞의 도우미 분들의 도움을 받고 오세요.", { prev = false, next = false })
            return
        end

        if step == 3 then
            local btn = me:dialog(npc, "짜잔! 놀라셨죠~! 저는 사도라고 합니다.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then return end

            btn = me:dialog(npc, "우선 아이템을 하나 드릴텐데요, 일단 받아보세요.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then return end

            if me:mkitem("목도", 1) == nil then
                me:dialog(npc, "소지품이 가득 차서 목도를 줄 수 없습니다.", { prev = false, next = false })
                return
            end
            q:step(4)
            q:param("0,0")

            me:dialog(npc, "제가 드린 아이템은 '목도'라는 아이템으로, 착용 가능한 무기 아이템 입니다.", { prev = false, next = true })
            me:dialog(npc, "알파벳 [i]를 누르시면, 받은 목도를 확인하실 수 있습니다. 클릭해서 착용해 보시겠어요?", { prev = false, next = false })
            return
        end

        if step == 4 then
            local w = me:weapon()
            local has_mokdo = (w and w:model() and w:model():name() == "목도")
            if has_mokdo then
                me:dialog(npc, "목도를 잘 착용하셨군요!! 참 잘하셨어요.", { prev = false, next = true })
                me:dialog(npc, "이처럼 목도와 같은 무기류를 착용하면, 더욱 강력해진답니다.", { prev = false, next = true })
                me:dialog(npc, "이제 사냥을 시작해볼까요?", { prev = false, next = true })
                me:dialog(npc, "불쌍하지만 귀여운 토끼와 다람쥐를 각각 5마리씩만 잡아보시겠어요?", { prev = false, next = true })
                me:dialog(npc, "토끼나 다람쥐 앞에 가서 알파벳 [a]나 [스페이스바]를 누르면 무기를 휘두를 수 있답니다.", { prev = false, next = true })
                me:dialog(npc, "5마리를 모두 잡으면 다시 저에게 오세요.", { prev = false, next = false })
                q:step(5)
            else
                me:dialog(npc, "목도를 착용 하셨나요? [i]키를 누르면 보이는 목도를 착용해 보세요.", { prev = false, next = false })
            end
            return
        end

        if step == 5 then
            local param = q:param() or ""
            local cnt_squirrel, cnt_rabbit = parse_hunt_param(param)
            if cnt_squirrel >= 5 and cnt_rabbit >= 5 then
                me:dialog(npc, "정말 잘 하셨어요! 모두 잡으셨네요.", { prev = false, next = true })
                me:dialog(npc, "이처럼 바람의나라의 세계에셔는, 몬스터를 잡고 경험치를 얻어 성장하실 수 있답니다.", { prev = false, next = true })
                me:dialog(npc, "그런데!!! 다람쥐와 토끼를 잡다 보면, 바닥에 뭔가가 떨어지곤 했습니다.", { prev = false, next = true })
                me:dialog(npc, "이렇게 몬스터를 해치우고 얻을 수 있는 것을 [드랍 아이템]이라고 합니다.", { prev = false, next = true })
                me:dialog(npc, "이러한 드랍 아이템인 [도토리]와 [토끼고기]를 각각 10개씩 모아보겠습니다. 부족한 아이템을 모아, 10개를 채워주세요.", { prev = false, next = false })
                q:step(6)
            else
                me:dialog(npc, string.format("토끼와 다람쥐를 각각 5마리씩 잡아보세요.\n다람쥐 : %d\n토끼   : %d", cnt_squirrel, cnt_rabbit), { prev = false, next = false })
            end
            return
        end

        if step == 6 then
            if me:has_items({ ["도토리"] = 10, ["토끼고기"] = 10 }) then
                me:dialog(npc, "와, 정말로 모두 모아오셨네요! 참 잘하셨어요.", { prev = false, next = true })
                me:dialog(npc, "지금 모아둔 도토리와 토끼고기는 나중에 쓸데가 있으니, 먹지 말고 모아두세요!", { prev = false, next = true })
                me:dialog(npc, "제가 드린 임무를 훌륭하게 수행하셨으니, 조그마한 선물을 드릴게요. 기쁘게 받아주셨으면 해요.", { prev = false, next = true })

                local armor = (me:gender() == GENDER.FEMALE) and "초보자용여자갑주" or "초보자용남자갑주"
                if me:mkitem({ [armor] = 1, ["도토리"] = 20, ["토끼고기"] = 20 }) == nil then
                    me:dialog(npc, "소지품이 가득 차서 선물을 줄 수 없습니다.", { prev = false, next = false })
                    return
                end
                me:exp(me:exp() + 300)
                q:step(7)
                me:dialog(npc, "이제 다음 방으로 이동하시면 됩니다. 제 윗쪽에 다음 건물의 입구가 있습니다.", { prev = false, next = true })
                me:dialog(npc, "그럼 안녕히가세요 ~~!", { prev = false, next = false })
            else
                local idotori = me:item("도토리")
                local imeat = me:item("토끼고기")
                local n_dotori = (idotori and idotori:count()) or 0
                local n_meat = (imeat and imeat:count()) or 0
                me:dialog(npc, string.format("도토리 (%d/10)개, 토끼고기 (%d/10)개 모으셨네요.\n[*TIP] 키보드의 쉼표 [,]키를 눌러 아이템을 주울 수 있다.", n_dotori, n_meat), { prev = false, next = false })
            end
            return
        end

        me:dialog(npc, "이제 다음 방으로 이동하시면 됩니다. 제 윗쪽에 다음 건물의 입구가 있습니다.", { prev = false, next = true })
        me:dialog(npc, "그럼 안녕히가세요 ~~!", { prev = false, next = false })
    end
}
