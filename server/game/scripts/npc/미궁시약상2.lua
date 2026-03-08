function NPC_306(me, npc)
::NPC_306_000::
    local selected = me:list(npc, '안녕하세요. 어떻게 오셨나요?', {
        '물건 사기', '물건 팔기'
    })
    if selected == nil then
        return
    end

    if selected == 0 then
        if NPC_SELL_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_306_000
        end
    elseif selected == 1 then
        if NPC_BUY_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_306_000
        end
    end
end
