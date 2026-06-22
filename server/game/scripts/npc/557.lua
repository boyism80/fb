-- npc: 도깨비상인
local npc = require('lib.npc')

function NPC_557(me, npc_obj)
::NPC_557_000::
    local selected = me:list(npc_obj, "난 비밀스러운 방망이를 파는 도깨비라네...", {"물건 사기"})
    if selected == nil then
        return
    end

    if selected == 0 then
        if npc.sell_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_557_000
        end
    end
end
