-- npc: 바람돌이
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        if not require('lib.festival').is('설날') then
            me:dialog(npc, "......", { prev = false, next = false })
            return
        end

        local button = me:dialog(npc, "설날의 겨울 바람은 너무 쌩쌩 불어요~! 그래서인지 바람개비를 가지고 놀면 추운것도 잊어버린다니까요~?", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "반년 전 세시마을에서는 창포물을 가지고 염색을 하곤 했는데, 그 때 조금 챙겨둔 것으로 바람개비를 물들여드릴 수 있어요.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, "하늘, 초록, 연두, 파랑, 분홍 바람개비를 각각 하나씩 가져오시면, 다른 색의 바람개비로 바꾸어 드릴게요.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        local t = datetime()
        if t.month ~= 2 or t.day < 4 or t.day > 6 then
            me:dialog(npc, "제가 가진것이 조금밖에 없어서, 2월 4일 ~ 2월 6일까지만 교환 해 드리니 그 때 바꾸러 오세요~!~!", { prev = false, next = false })
            return
        end

        local sel, list_btn = me:list(npc, "하늘, 초록, 연두, 파랑, 분홍 바람개비를 가져오시면, 다른 색의 바람개비로 바꾸어 드릴게요.", {
            "가져왔는데 바꾸어주겠니?",
            "다섯개를 하나로 바꾼다니, 너무하잖아!"
        }, { prev = true })
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if list_btn == DIALOG_RESULT.PREV then
            return
        end
        if sel == nil then
            return
        end

        if sel == 2 then
            me:dialog(npc, "저도 소중한 창포물과, 염색을 하려고 따둔 꽃을 힘들게 구했단 말이에요! 싫으면 마세요!", { prev = false, next = false })
            return
        end

        local cost = { ['item'] = { ["하늘개비"] = 1, ["초록개비"] = 1, ["연두개비"] = 1, ["파랑개비"] = 1, ["분홍개비"] = 1 } }
        local r = math.random(1, 100)
        local reward_name
        if r <= 5 then
            reward_name = (math.random(1, 2) == 1) and "얼음개비" or "눈꽃개비"
        elseif r < 25 then
            reward_name = (math.random(1, 2) == 1) and "노랑개비" or "빨강개비"
        else
            local names = { "보라개비", "진청개비", "진녹개비" }
            reward_name = names[math.random(1, 3)]
        end
        local reward = { ['item'] = { [reward_name] = 1 } }
        local code = me:exchange(cost, reward)
        if code == enum.exchange_result.LACK_COST then
            me:dialog(npc, "다섯 색깔의 모든 바람개비를 가져오셔야 해요.", { prev = false, next = false })
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 바람개비를 받을 수 없어요.", { prev = false, next = false })
            return
        end
    end
}
