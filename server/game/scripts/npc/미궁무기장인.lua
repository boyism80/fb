
function NPC_307(me, npc)
    local d = me:dialog(npc, "비밀의 상점에 온 것을 환영하네. 여기선 방천화극을 팔고 있지...", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "미궁에서 혼자 지내려니 무엇보다 술이 고프더군... 혹 작약주가 있다면 무기를 내어줌세...", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end
    d = me:dialog(npc, "신작약주가 있다면, 전사 전용의 신무기까지 만들어 줄 수 있네만...", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end

    local sel, btn = me:list(npc, "작약주가 있는가?", {
        "예 작약주가 있습니다.",
        "예 신작약주가 있습니다.",
        "작약주가 없습니다.",
        "용궁방천화극",
    }, false)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 0 then
        local code = me:exchange(
            { ['item'] = { ["작약주"] = 1 } },
            { ['item'] = { ["방천화극"] = 1 } }
        )
        if code == EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "뭐야? 아무것도 없질 않은가!!", false, false)
            return
        end
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 방천화극을 드리지 못합니다.", false, false)
            return
        end
        me:dialog(npc, "작약주를 가지고 있구만... 예전엔 참 좋은 술이었는데...\n\n여기 방천화극이 있네. 종종 들리게나...", false, false)
        return
    end

    if sel == 1 then
        local code = me:exchange(
            { ['item'] = { ["신작약주"] = 1 } },
            { ['item'] = { ["전사방천화극"] = 1 } }
        )
        if code == EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "뭐야? 아무것도 없질 않은가!!", false, false)
            return
        end
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 전사방천화극을 드리지 못합니다.", false, false)
            return
        end
        me:dialog(npc, "신작약주를 가지고 있구만... 예전엔 참 좋은 술이었는데...\n\n여기 방천화극이 있네. 종종 들리게나...", false, false)
        return
    end

    if sel == 2 then
        me:dialog(npc, "잘 가게나...", false, false)
        return
    end

    if sel == 3 then
        local q = me:quest(QUEST_CIDEQUEST)
        local pn = (q and q:progress()) or 0
        if pn == 2 then
            local l1, b1 = me:list(npc, " ", { "이봐요.", "당신이 용왕님의 보물을 가져갔다고 하던데.." }, false)
            if b1 == DIALOG_RESULT.QUIT or l1 == nil then
                return
            end
            local l2, b2 = me:list(npc, "아니 난 그런 적 없소.", { "장돌뱅이한테 물어보니까 당신이", "가지고 가는 것을 보았다고 하는데", "끝까지 발뺌할 건가요?" }, false)
            if b2 == DIALOG_RESULT.QUIT or l2 == nil then
                return
            end
            local l3, b3 = me:list(npc, "앗. 설마 그 친구가 배반할 줄은 몰랐는걸.", { "..." }, false)
            if b3 == DIALOG_RESULT.QUIT or l3 == nil then
                return
            end
            if me:mkitem("용궁의보물", 1) == nil then
                me:dialog(npc, "소지품이 가득 차서 용궁의보물을 드리지 못합니다.", false, false)
                return
            end
            if q == nil then
                q = me:start_quest(QUEST_CIDEQUEST)
                if q == nil then
                    return
                end
            end
            if q then
                q:step(2)
                q:progress(3)
            end
            me:dialog(npc, "옛소. 이걸 받는 대신 내가 가져갔다는 말 하면 안되오.", false, false)
            return
        end
        if pn >= 3 then
            me:dialog(npc, "아무한테도 말하지 말아주시오.", false, false)
            return
        end
        me:dialog(npc, "그건 왜 찾는가?", false, false)
    end
end
