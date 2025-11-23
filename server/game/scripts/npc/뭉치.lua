function NPC_15(me, npc)
::NPC_15_000::
    local selected = me:list(npc, '안녕하세요. 무엇을 도와드릴까요?', {'물건 사기', '물건 팔기'})
    if selected == nil then
        return
    end

    if selected == 0 then
        if NPC_SELL_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_15_000
        end
    elseif selected == 1 then
        if NPC_BUY_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_15_000
        end
    else

    end
end