local npc = require('lib.npc')

function NPC_14(me, npc_obj)
::NPC_14_000::
    if not npc.cloth_shop_dialog(me, npc_obj, nil) then
        return
    end
    goto NPC_14_000
end