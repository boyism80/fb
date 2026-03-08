function NPC_557(me, npc)
::NPC_557_000::
    local selected = me:list(npc, "난 비밀스러운 방망이를 파는 도깨비라네...", {"물건 사기"})
    if selected == nil then
        return
    end

    if selected == 0 then
        if NPC_SELL_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_557_000
        end
    end
end
