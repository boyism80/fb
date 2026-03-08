function NPC_390(me, npc)
::NPC_390_000::
    local selected = me:list(npc, "부모님께 카네이션을 선물하고 싶으시다구요?", {"꽃을 산다"})
    if selected == nil then
        return
    end

    if selected == 0 then
        if NPC_SELL_DIALOG(me, npc) == DIALOG_RESULT.NEXT then
            goto NPC_390_000
        end
    end
end
