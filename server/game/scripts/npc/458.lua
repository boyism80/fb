-- npc: 산타클로스
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        local sel, btn = me:list(npc, "안녕하세요. 어떻게 오셨나요?", { "홍성초를 가져왔어요.", "산타양말" }, { prev = false })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil then
            return
        end

        if sel == 1 then
            local d = me:dialog(npc, "이번 크리스마스 트리를 장식할 홍성초가 부족한데... 어떻게 해야 한담...", { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            local sub, sub_btn = me:list(npc, "이걸 어떻게 해야 좋을까...", { "홍성초를 가져왔어요.", "힘내세요!" }, { prev = false })
            if sub_btn == DIALOG_RESULT.QUIT then
                return
            end
            if sub == 1 then
                local code = me:exchange(
                    { ['item'] = { ["홍성초"] = 1 } },
                    { ['item'] = { ["빨간양말"] = 1 } }
                )
                if code == enum.exchange_result.LACK_COST then
                    me:dialog(npc, "홍성초가 없는데?", { prev = false, next = false })
                    return
                end
                if code == enum.exchange_result.LACK_CAPACITY then
                    me:dialog(npc, "소지품이 가득 차서 빨간양말을 드리지 못합니다.", { prev = false, next = false })
                    return
                end
                me:dialog(npc, "정말 고맙네. 답례로 소소한 선물이네. 메리 크리스마스~", { prev = false, next = false })
            else
                me:dialog(npc, "고맙네, 허허. 그나저나 이를 어쩌지...", { prev = false, next = false })
            end
            return
        end

        if sel == 2 then
            local d = me:dialog(npc, "아니 글쎄... 빨간양말에 선물을 담아 나누어 주어야 하는데, 그만 빨간양말을 잃어버렸지 뭔가! 이걸 어떻게 해야 한담...", { prev = false, next = true })
            if d == DIALOG_RESULT.QUIT then
                return
            end
            local sub, sub_btn = me:list(npc, "이걸 어떻게 해야 좋을까...", {
                "빨간양말 100개를 가져왔어요.",
                "빨간양말 200개를 가져왔어요.",
            }, { prev = false })
            if sub_btn == DIALOG_RESULT.QUIT then
                return
            end
            if sub == nil then
                return
            end
            local need = (sub == 1) and 100 or 200
            local reward_name = (sub == 1) and "산타모자" or "산타클로스옷"
            local code = me:exchange(
                { ['item'] = { ["빨간양말"] = need } },
                { ['item'] = { [reward_name] = 1 } }
            )
            if code == enum.exchange_result.LACK_COST then
                me:dialog(npc, "안그래도 심란한데... 거짓말 말게나!!", { prev = false, next = false })
                return
            end
            if code == enum.exchange_result.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 " .. reward_name .. "을(를) 드리지 못합니다.", { prev = false, next = false })
                return
            end
            me:dialog(npc, "정말 고맙네, 허허. 답례로 소소한 선물이네. 메리 크리스마스~", { prev = false, next = false })
        end
    end
}
