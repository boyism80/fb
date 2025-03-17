function NPC_9(me, npc)
::NPC_9_000::
    local selected = me:list(npc, '안녕하세요. 무엇을 도와드릴까요?', {'물건 사기', '물건 팔기', '물건 고치기', '모두 고치기', '무기 이름 명명'})
    if selected == nil then
        return
    end

    if selected == 0 then
        if NPC_SELL_DIALOG(me, npc) == DIALOG_RESULT_NEXT then
            goto NPC_9_000
        end
    elseif selected == 1 then
        if NPC_BUY_DIALOG(me, npc) == DIALOG_RESULT_NEXT then
            goto NPC_9_000
        end
    elseif selected == 2 then
        if NPC_REPAIR_DIALOG(me, npc) == DIALOG_RESULT_NEXT then
            goto NPC_9_000
        end
    elseif selected == 3 then
        if NPC_REPAIR_ALL_DIALOG(me, npc) == DIALOG_RESULT_NEXT then
            goto NPC_9_000
        end
    elseif selected == 4 then
    	if NPC_RENAME_WEAPON_DIALOG(me, npc) == DIALOG_RESULT_NEXT then
           goto NPC_9_000 
        end
    else

    end
end