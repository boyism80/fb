function NPC_254(me, npc)
::NPC_254_000::
    local selected = me:list(npc, '안녕하세요. 어떻게 오셨나요?', {
        '물건 사기',
        '물건 팔기',
        '금전 맡기기',
        '금전 되찾기',
        '물건 맡기기',
        '물건 되찾기'
    })
    if selected == nil then
        return
    end

    if selected == 0 then
        if NPC_SELL_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_254_000
        end
    elseif selected == 1 then
        if NPC_BUY_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_254_000
        end
    elseif selected == 2 then
        if NPC_HOLD_MONEY_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_254_000
        end
    elseif selected == 3 then
        if NPC_RETURN_MONEY_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_254_000
        end
    elseif selected == 4 then
        if NPC_HOLD_ITEM_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_254_000
        end
    elseif selected == 5 then
        if NPC_RETURN_ITEM_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_254_000
        end
    end
end