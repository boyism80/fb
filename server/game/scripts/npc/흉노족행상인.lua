local npc = require('lib.npc')

function NPC_488(me, npc_obj)
::NPC_488_000::
    local selected = me:list(npc_obj, "이 위험한 곳까지 와서 장사하기가 쉬운 줄 아나?", {"물건 사기", "물건 팔기"})
    if selected == nil then
        return
    end

    if selected == 0 then
        if npc.sell_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_488_000
        end
    elseif selected == 1 then
        if npc.buy_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_488_000
        end
    end
end
