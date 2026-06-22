-- npc: 흉노상점2
local npc = require('lib.npc')

function NPC_201(me, npc_obj)
::NPC_201_000::
    local selected = me:list(npc_obj, '안녕하세요. 무엇을 도와드릴까요?', {'물건 사기', '물건 팔기'})
    if selected == nil then
        return
    end

    if selected == 0 then
        if npc.sell_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_201_000
        end
    elseif selected == 1 then
        if npc.buy_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_201_000
        end
    else

    end
end
