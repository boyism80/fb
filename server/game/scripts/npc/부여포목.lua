local AMBER_CRYSTAL_EXCHANGE = {
    { source = { name = '연갈호박단추', count = 15 }, dest = { name = '연갈호박결정', count = 1 } },
    { source = { name = '연녹호박단추', count = 15 }, dest = { name = '연녹호박결정', count = 1 } },
    { source = { name = '연자호박단추', count = 15 }, dest = { name = '연자호박결정', count = 1 } },
    { source = { name = '연청호박단추', count = 15 }, dest = { name = '연청호박결정', count = 1 } }
}

function NPC_232(me, npc)
::NPC_232_000::
    if not npc_cloth_shop_dialog(me, npc, AMBER_CRYSTAL_EXCHANGE) then
        return
    end
    goto NPC_232_000
end