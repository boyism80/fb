function NPC_547(me, npc)
::NPC_547_000::
    local selected = me:list(npc, ".....", {"물건 팔기"})
    if selected == nil then
        return
    end

    if selected == 0 then
        if NPC_BUY_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_547_000
        end
    end
end
