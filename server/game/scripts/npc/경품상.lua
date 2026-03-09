
local function run_wedding_dress_sale(me, npc)
    local btn = me:dialog(npc, "안녕하세요? 특별 이벤트 기간중에 희귀 웨딩드레스를 판매하고 있습니다.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    local sel, list_btn = me:list(npc, "아래의 웨딩드레스 중 원하는 것을 골라보세요.", {
        "웨딩드레스14를 사고 싶어요.",
        "웨딩드레스12를 사고 싶어요.",
        "웨딩드레스18을 사고 싶어요.",
        "웨딩드레스22를 사고 싶어요.",
        "웨딩드레스25를 사고 싶어요."
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel == 0 then
        btn = me:dialog(npc, "웨딩드레스14의 가격은 200만전입니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then return end
        local buy_sel, buy_btn = me:list(npc, "구매하시겠어요?", {"네, 사겠습니다.", "아니오, 다음에요..."}, false)
        if buy_btn == DIALOG_RESULT.QUIT or buy_sel == nil then return end
        if buy_sel == 0 then
            if me:money() < 2000000 then
                me:dialog(npc, "돈이 부족하시네요.", false, false)
                return
            end
            me:money(me:money() - 2000000)
            me:mkitem("웨딩드레스14", 1)
            me:dialog(npc, "여기 웨딩드레스14입니다. 감사합니다.", false, true)
        else
            me:dialog(npc, "다음에 또 방문해주세요.", false, false)
        end
        return
    end
    if sel == 1 then
        btn = me:dialog(npc, "웨딩드레스12의 가격은 5000만전입니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then return end
        local buy_sel, buy_btn = me:list(npc, "구매하시겠어요?", {"네, 사겠습니다.", "아니오, 다음에요..."}, false)
        if buy_btn == DIALOG_RESULT.QUIT or buy_sel == nil then return end
        if buy_sel == 0 then
            if me:money() < 50000000 then
                me:dialog(npc, "돈이 부족하시네요.", false, false)
                return
            end
            me:money(me:money() - 50000000)
            me:mkitem("웨딩드레스12", 1)
            me:dialog(npc, "여기 웨딩드레스12입니다. 감사합니다.", false, true)
        else
            me:dialog(npc, "다음에 또 방문해주세요.", false, false)
        end
        return
    end
    if sel == 2 then
        btn = me:dialog(npc, "웨딩드레스18의 가격은 1억전입니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then return end
        local buy_sel, buy_btn = me:list(npc, "구매하시겠어요?", {"네, 사겠습니다.", "아니오, 다음에요..."}, false)
        if buy_btn == DIALOG_RESULT.QUIT or buy_sel == nil then return end
        if buy_sel == 0 then
            if me:money() < 100000000 then
                me:dialog(npc, "돈이 부족하시네요.", false, false)
                return
            end
            me:money(me:money() - 100000000)
            me:mkitem("웨딩드레스18", 1)
            me:dialog(npc, "여기 웨딩드레스18입니다. 감사합니다.", false, true)
        else
            me:dialog(npc, "다음에 또 방문해주세요.", false, false)
        end
        return
    end
    if sel == 3 then
        btn = me:dialog(npc, "웨딩드레스12와 웨딩드레스18을 가져오시면 웨딩드레스22로 교환해드립니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then return end
        local ex_sel, ex_btn = me:list(npc, "교환하시겠어요?", {"네, 교환하겠습니다.", "아니오, 다음에요..."}, false)
        if ex_btn == DIALOG_RESULT.QUIT or ex_sel == nil then return end
        if ex_sel == 0 then
            local code = me:exchange(
                { ['item'] = { ["웨딩드레스12"] = 1, ["웨딩드레스18"] = 1 } },
                { ['item'] = { ["웨딩드레스22"] = 1 } }
            )
            if code == EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, "웨딩드레스가 없으신데요?", false, false)
                return
            end
            if code == EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 웨딩드레스22를 드리지 못합니다.", false, false)
                return
            end
            me:dialog(npc, "여기 웨딩드레스22입니다. 감사합니다.", false, true)
        else
            me:dialog(npc, "다음에 또 방문해주세요.", false, false)
        end
        return
    end
    if sel == 4 then
        btn = me:dialog(npc, "웨딩드레스18과 웨딩드레스22를 가져오시면 웨딩드레스25로 교환해드립니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then return end
        local ex_sel, ex_btn = me:list(npc, "교환하시겠어요?", {"네, 교환하겠습니다.", "아니오, 다음에요..."}, false)
        if ex_btn == DIALOG_RESULT.QUIT or ex_sel == nil then return end
        if ex_sel == 0 then
            local code = me:exchange(
                { ['item'] = { ["웨딩드레스18"] = 1, ["웨딩드레스22"] = 1 } },
                { ['item'] = { ["웨딩드레스25"] = 1 } }
            )
            if code == EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, "웨딩드레스가 없으신데요?", false, false)
                return
            end
            if code == EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, "소지품이 가득 차서 웨딩드레스25를 드리지 못합니다.", false, false)
                return
            end
            me:dialog(npc, "여기 웨딩드레스25입니다. 감사합니다.", false, true)
        else
            me:dialog(npc, "다음에 또 방문해주세요.", false, false)
        end
        return
    end
end

local function run_garimbin_wing_exchange(me, npc)
    ::dialog_loop::
    local btn = me:dialog(npc, "안녕하세요? 특별 이벤트 기간중에 웨딩드레스25와 천사의깃털 10개를 가릉빈가의날개옷으로 교환해드려요.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    local sel, list_btn = me:list(npc, "교환하시겠어요?", {"네, 교환하겠습니다.", "아니오, 다음에요..."}, true)
    if list_btn == DIALOG_RESULT.PREV then
        goto dialog_loop
    end
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel == 0 then
        local code = me:exchange(
            { ['item'] = { ["웨딩드레스25"] = 1, ["천사의깃털"] = 10 } },
            { ['item'] = { ["가릉빈가의날개옷"] = 1 } }
        )
        if code == EXCHANGE_RESULT.LACK_COST then
            if not me:has_items("웨딩드레스25", 1) then
                me:dialog(npc, "웨딩드레스가 없으신데요?", false, false)
            else
                me:dialog(npc, "천사의깃털이 부족하신데요?", false, false)
            end
            return
        end
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 가릉빈가의날개옷을 드리지 못합니다.", false, false)
            return
        end
        me:dialog(npc, "여기 가릉빈가의날개옷입니다. 감사합니다.", false, true)
    else
        me:dialog(npc, "다음에 또 방문해주세요.", false, false)
    end
end

function NPC_490(me, npc)
    local sel, btn = me:list(npc, "안녕하세요. 어떻게 오셨나요?", {
        "웨딩드레스특별판매",
        "가릉빈가의날개옷교환"
    })
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end
    if sel == 0 then
        run_wedding_dress_sale(me, npc)
        return
    end
    if sel == 1 then
        run_garimbin_wing_exchange(me, npc)
        return
    end
end
