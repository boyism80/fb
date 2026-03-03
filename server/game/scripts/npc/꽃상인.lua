function NPC_552(me, npc)
::NPC_552_000::
    local selected = me:list(npc, "안녕하세요. 어떻게 오셨나요? 왠지 오늘은 장사가 잘 될 것 같은데...", {"물건 사기", "물건 팔기"})
    if selected == nil then
        return
    end

    if selected == 0 then
        if NPC_SELL_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_552_000
        end
    elseif selected == 1 then
        if NPC_BUY_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_552_000
        end
    end
end
