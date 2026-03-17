
function NPC_355(me, npc)
    if gv("sesi_rightnow") ~= 5 then
        local btn = me:dialog(npc, "안녕하세요? 저는 오단미입니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    local sel, list_btn = me:list(npc, "제가 도와드릴 일이 있나요?", {
        "단오엔 무슨 일을 하나요?",
        "창포를 가지고 왔어요.",
        "비녀를 만들어 주세요.",
        "머리를 감고 왔어요.",
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 0 then
        local btn = me:dialog(npc, "음력 5월 5일은 단오에요.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "단오는 우리나라 3대 명절중 하나에요.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "남자들은 씨름 대회를 열고, 여자들은 그네를 뛰면서 즐겁게 노는 날이죠.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "단오에 비가 오면 그 해에 풍년이 든다는 이야기가 있는데.. 이번 단오에 비가 오려나...", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "아, 참! 단오에 창포물로 머리 감는건 알고 계시죠?", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "창포를 구해오시면 제가 창포물을 만들어 드릴께요.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "저희 집 마당의 우물에서 머리를 감으세요.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    if sel == 1 then
        local btn = me:dialog(npc, "창포를 구해오셨나요?", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "어디보자....", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local reward = nil
        local r = math.random(1, 3)
        if r == 1 then
            reward = { ['item'] = { ["창포뿌리"] = 1 } }
        elseif r == 3 then
            reward = { ['item'] = { ["창포물"] = 1 } }
        end
        local code = me:exchange(
            { ['item'] = { ["창포"] = 1 } },
            reward
        )
        if code == EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "창포를 가지고 오세요.", false, false)
            return
        end
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 받을 수 없어요. 자리 좀 비우고 다시 오세요.", false, false)
            return
        end
        if reward and reward['item']["창포뿌리"] then
            btn = me:dialog(npc, "이 창포는 뿌리가 매우 좋네요.. 창포물 만들긴 좀 아까운데요?", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "창포뿌리를 다섯개 모아오시면 제가 비녀를 만들어 드리죠.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
        elseif reward and reward['item']["창포물"] then
            btn = me:dialog(npc, "이 창포로 창포물을 만들어 드릴께요..", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "그럼 저희집 마당의 우물가에서 머리를 감으세요. 그러면 머리카락에 윤기가 생기고 빠지지 않게 된답니다.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
        else
            btn = me:dialog(npc, "어머, 이건 썩은 창포잖아요.. 이걸로는 창포물을 만들수가 없어요..", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
        end
        return
    end

    if sel == 2 then
        local btn = me:dialog(npc, "창포뿌리를 다섯개나 모으셨어요? 그럼 제가 비녀를 만들어 드릴께요.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "매달마다 받으시는 아이템을 다 모으시면 좋은일이 생길테니.. 잘 보관하세요. 단오의 아이템은 비녀에요.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local code = me:exchange(
            { ['item'] = { ["창포뿌리"] = 5 } },
            { ['item'] = { ["비녀"] = 1 } }
        )
        if code == EXCHANGE_RESULT.LACK_COST then
            btn = me:dialog(npc, "창포뿌리 다섯개를 가져오시면 비녀를 만들어 드릴게요.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            return
        end
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 비녀를 받을 수 없어요. 자리 좀 비우고 다시 오세요.", false, false)
            return
        end
        me:dialog(npc, "창포뿌리 다섯개 받았어요. 비녀 만들어 드렸어요.", false, false)
        return
    end

    if sel == 3 then
        if me:isbuff("반짝반짝") then
            me:mkitem("망개떡", 1)
            btn = me:dialog(npc, "제가 떡을 준비했는데.. 배고플때 드세요. ^^", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "망개떡을 드릴께요.. 맛있게 드세요~~", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
        else
            btn = me:dialog(npc, "밖의 우물에서 머리를 감고 오세요.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
        end
    end
end
