-- npc: 금권
local npc = require('lib.npc')

function NPC_376(me, npc_obj)
::NPC_376_000::
    local selected = me:list(npc_obj, "안녕하세요. 어떻게 오셨나요?", {"물건 사기", "물건 팔기"})
    if selected == nil then
        return
    end

    if selected == 0 then
        local button = me:dialog(npc_obj, "살 수는 없소.", false, false)
        if button == DIALOG_RESULT.QUIT then
            return
        end
    elseif selected == 1 then
        if npc.buy_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_376_000
        end
    end
end
