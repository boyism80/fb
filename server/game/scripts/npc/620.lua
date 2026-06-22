-- npc: 백두촌대장장이
local npc = require('lib.npc')

function NPC_620(me, npc_obj)
    ::NPC_620_000::
        local selected = me:list(npc_obj, '안녕하세요. 무엇을 도와드릴까요?', {'물건 사기', '물건 팔기', '물건 고치기', '모두 고치기', '무기 이름 명명'})
        if selected == nil then
            return
        end
    
        if selected == 0 then
            if npc.sell_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
                goto NPC_620_000
            end
        elseif selected == 1 then
            if npc.buy_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
                goto NPC_620_000
            end
        elseif selected == 2 then
            if npc.repair_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
                goto NPC_620_000
            end
        elseif selected == 3 then
            if npc.repair_all_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
                goto NPC_620_000
            end
        elseif selected == 4 then
            if npc.rename_weapon_dialog(me, npc_obj) == DIALOG_RESULT.NEXT then
               goto NPC_620_000 
            end
        else
    
        end
    end
