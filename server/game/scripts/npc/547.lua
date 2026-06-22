-- npc: 암상인
local npc = require('lib.npc')

function NPC_547(me, npc_obj)
::NPC_547_000::
    local selected = me:list(npc_obj, ".....", {"물건 팔기"})
    if selected == nil then
        return
    end

    if selected == 0 then
        if npc.buy_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_547_000
        end
    end
end
