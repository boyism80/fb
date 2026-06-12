
function NPC_576(me, npc)
    if property("goldarcon") ~= 36 then
        me:dialog(npc, "지금은 때가 아닙니다.", false, false)
        return
    end

    local btn = me:dialog(npc, "안녕하세요? 저는 [황금도토리]를 모으고 있는 [토로라비]입니다. 톨루이와는 오누이 지간입니다.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local sel, list_btn = me:list(npc, "가지고 계신 황금도토리를 [특별교환권]과 [일반교환권]으로 바꾸어 드립니다. 어떻게 하시겠어요?", {
        "황금도토리 1개 : 일반교환권",
        "황금도토리 6개 : 특별교환권",
        "다음에는 언제오니...?",
    }, false)
    if list_btn == DIALOG_RESULT.QUIT or sel == nil then
        return
    end

    if sel == 0 then
        local code = me:exchange(
            { ['item'] = { ["황금도토리"] = 1 } },
            { ['item'] = { ["일반교환권"] = 1 } }
        )
        if code == EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "황금도토리가 없으시군요. 황금도토리를 가져오세요.", false, false)
            return
        end
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 일반교환권을 받을 수 없어요.", false, false)
            return
        end
        me:dialog(npc, "황금도토리를 일반교환권으로 교환해 드렸어요.", false, false)
        return
    end

    if sel == 1 then
        local code = me:exchange(
            { ['item'] = { ["황금도토리"] = 6 } },
            { ['item'] = { ["특별교환권"] = 1 } }
        )
        if code == EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "황금도토리가 없으시군요. 황금도토리를 가져오세요.", false, false)
            return
        end
        if code == EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 특별교환권을 받을 수 없어요.", false, false)
            return
        end
        me:dialog(npc, "황금도토리를 특별교환권으로 교환해 드렸어요.", false, false)
        return
    end

    if sel == 2 then
        me:dialog(npc, "저는 세시간에 한번 찾아온답니다. 제가 사라지면, 세시간 뒤에 저를 다시 찾아주세요.", false, false)
    end
end
