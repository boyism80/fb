local npc = require('lib.npc')

function NPC_233(me, npc_obj)
::NPC_233_000::
    if not npc.armor_shop_dialog(me, npc_obj) then
        return
    end
    goto NPC_233_000
end