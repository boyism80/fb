local npc = require('lib.npc')

function NPC_128(me, npc_obj)
::NPC_128_000::
    local selected = me:list(npc_obj, '안녕하세요. 무엇을 도와드릴까요?', {'물건 사기', '물건 팔기'})
    if selected == nil then
        return
    end

    if selected == 0 then
        if npc.sell_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_128_000
        end
    elseif selected == 1 then
        if npc.buy_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_128_000
        end
    else

    end
end