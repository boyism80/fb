local npc = require('lib.npc')

function NPC_255(me, npc_obj)
::NPC_255_000::
    local selected = me:list(npc_obj, '안녕하세요. 어떻게 오셨나요?', {
        '물건 사기',
        '물건 팔기',
        '금전 맡기기',
        '금전 되찾기',
        '물건 맡기기',
        '물건 되찾기'
    })
    if selected == nil then
        return
    end

    if selected == 0 then
        if npc.sell_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_255_000
        end
    elseif selected == 1 then
        if npc.buy_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_255_000
        end
    elseif selected == 2 then
        if npc.hold_money_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_255_000
        end
    elseif selected == 3 then
        if npc.return_money_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_255_000
        end
    elseif selected == 4 then
        if npc.hold_item_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_255_000
        end
    elseif selected == 5 then
        if npc.return_item_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
            goto NPC_255_000
        end
    end
end