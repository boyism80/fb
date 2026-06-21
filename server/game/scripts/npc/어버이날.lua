local npc = require('lib.npc')

function NPC_390(me, npc_obj)
::NPC_390_000::
    local selected = me:list(npc_obj, "부모님께 카네이션을 선물하고 싶으시다구요?", {"꽃을 산다"})
    if selected == nil then
        return
    end

    if selected == 0 then
        if npc.sell_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_390_000
        end
    end
end
