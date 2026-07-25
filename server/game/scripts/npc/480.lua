-- npc: 한수
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        if not me:has_items({ ["한왕"] = 1, ["초왕"] = 1 }) then
            local button = me:dialog(npc, "초패 저 친구와 장기를 둬야 하는데, 원숭이 놈들이 장기알을 훔쳐가버렸지 뭐야...", { prev = false, next = false })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            return
        end

        local button = me:dialog(npc, "오, 장기알 왕이 있구만, 그걸 내게 주겠는가? 싫다면 지금 그만두게...", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        local cost = { ['item'] = { ["한왕"] = 1, ["초왕"] = 1 } }
        local reward = nil
        if math.random(1, 2) == 1 then
            reward = { ['item'] = { ["황매주"] = 1 } }
        end
        local code = me:exchange(cost, reward)
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            local btn = me:dialog(npc, "님 아이템복사 버그로 영정임", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            return
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 황매주를 받을 수 없네.", { prev = false, next = true })
            return
        end
        if reward ~= nil then
            button = me:dialog(npc, "오늘은 내가 운이 좋나보네 그려. 장기알도 얻고, 장기판에서도 이겼으니 말일세...", { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            button = me:dialog(npc, "내 자네에게 이 술을 줌세. 마시면 기분이 좋을 걸세...", { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            button = me:dialog(npc, "내 바둑 친구가 이 술을 좋아 했었는데...", { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            button = me:dialog(npc, "하여간 이제 자네에게 주겠네...", { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
        else
            button = me:dialog(npc, "오늘도 져버렸군... 다음엔 이겨야 할텐데...", { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
        end
    end
}
