function NPC_14(me, npc)
::NPC_14_000::
    if not npc_cloth_shop_dialog(me, npc, nil) then
        return
    end
    goto NPC_14_000
end