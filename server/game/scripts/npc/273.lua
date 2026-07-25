-- npc: 마타제자
local quest = require('lib.quest')
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        local q = me:quest(quest.QUEST_PAMASPIRI)
        if q == nil then
            ::NPC_273_0001::
            local sel, btn = me:list(npc, "아니, 이게 어디로 갔나... 스승님께 들키기라도 하면 경을 칠 텐데, 큰일이네. 깜빡 졸았던 사이에 누가 빼 갔나? 이를 어쩌지?", { "무슨 잃어버린 물건이라도 있습니까?", "난 가던 길이나 가야지..." }, { prev = false })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if sel == nil or sel == 2 then
                return
            end

            ::NPC_273_0010::
            local button = me:dialog(npc, "예? 음...음... 물건을 하나 잃어버린 것 같은데 통 보이질 않는군요. 겉보기엔 보통 피리인데......", { prev = true, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if button == DIALOG_RESULT.PREV then
                goto NPC_273_0001
            end

            ::NPC_273_0011::
            button = me:dialog(npc, "그 피리는 '파마의피리'라고, 제 스승님께서 귀신을 퇴치하실 때 쓰시는 중요한 물건이지요. 그런데 그게 제가 깜빡 존 사이에......", { prev = true, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if button == DIALOG_RESULT.PREV then
                goto NPC_273_0010
            end

            ::NPC_273_0012::
            sel, btn = me:list(npc, "누가 가져갔는지, 아니면 제가 어디에 떨어뜨렸는지 통 보이질 않네요. 으으... 그 중요한 물건을 잃어버린 것을 스승님께 들키는 날이면...", { "저도 찾는 것을 돕겠습니다.", "그러게 평소에 조심하셨어야죠. 전 이만..." }, { prev = false })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_273_0011
            end
            if sel == nil or sel == 2 then
                me:dialog(npc, "하긴 제 잘못이겠죠...", { prev = false, next = false })
                return
            end

            q = me:start_quest(quest.QUEST_PAMASPIRI)
            if q == nil then
                me:dialog(npc, "퀘스트를 시작할 수 없습니다.", { prev = false, next = false })
                return
            end
            if q then
                q:step(1)
            end
            me:push_achievement(48, "파마의 피리를 찾자.", 7, 1)

            ::NPC_273_0020::
            button = me:dialog(npc, "예? 정말 그래도 되겠습니까? 이렇게 고마울 데가...", { prev = true, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if button == DIALOG_RESULT.PREV then
                goto NPC_273_0012
            end

            ::NPC_273_0021::
            button = me:dialog(npc, "음, 그럼 이 근처를 먼저 좀 뒤져 주십시오. 사람들이 있으면 피리를 보지 못했느냐고 좀 물어봐도 주시고요. 저는 자리를 비우면 스승님께 들킬까봐 여길 떠날 수가 없네요...", { prev = true, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if button == DIALOG_RESULT.PREV then
                goto NPC_273_0020
            end

            ::NPC_273_0022::
            me:dialog(npc, "아직 스승님께 들키진 않았습니다만 만약 들키기라도 하면... 으으, 불안하군요. 일단 이 부근부터 좀 뒤져 주십시오.", { prev = false, next = false })
            return
        end

        if q:completed() then
            me:dialog(npc, string.format("%s님 덕분에 스승님께 들키지 않고 파마의피리를 원래 자리에 돌려놓을 수 있었습니다. 다신 잃어버리지 않게 조심해야죠.", me:name()), { prev = false, next = false })
            return
        end

        local step = q:step()
        if step == 4 then
            if not me:has_items("파마의피리", 1) then
                me:dialog(npc, "파마의피리를 가져오시면 마반경으로 바꿔드리겠네요.", { prev = false, next = false })
                return
            end

            ::NPC_273_0030::
            local button = me:dialog(npc, "앗! 파마의피리를 찾아오셨군요! 고맙습니다. 덕분에 살았습니다.", { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end

            ::NPC_273_0031::
            button = me:dialog(npc, "사례로 '마반경'을 드리죠. 이것은 사악한 힘으로부터 몸을 지켜주는 신기한 물건입니다.", { prev = false, next = true })
            if button == DIALOG_RESULT.PREV then
                goto NPC_273_0030
            end
            if button == DIALOG_RESULT.QUIT then
                return
            end

            local code = me:exchange(
                { ['item'] = { ["파마의피리"] = 1 } },
                { ['item'] = { ["마반경"] = 1 } }
            )
            if code == enum.EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, "파마의피리가 없어 보이네요.", { prev = false, next = false })
                return
            elseif code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 줄 수 없네요.", { prev = false, next = false })
                return
            end

            q:complete()
            me:push_achievement(48, "파마의 피리를 찾아주었다!", 7, 1)

            me:dialog(npc, "어이쿠, 스승님께 들키키 전에 파마의피리를 원래 자리에다 돌려놔야죠. 그럼 전 이만...!", { prev = false, next = false })
            return
        end

        me:dialog(npc, "스승님에게 들키기 전에 어서 파마의피리를 찾아주세요.", { prev = false, next = false })
    end
}
