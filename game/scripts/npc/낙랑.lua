function on_interact(me, npc)
::ROUTINE_1::
    local selected = npc:menu(me, '안녕하세요. 무엇을 도와드릴까요?', {'물건 사기', '물건 팔기', '물건 고치기', '모두 고치기', '금전 맡기기', '물건 맡기기', '금전 찾기', '물건 찾기', '무기 이름 명명'})
    if selected == nil then
        return
    end

    if selected == 0 then
        if npc:sell(me) == DIALOG_RESULT_NEXT then
            goto ROUTINE_1
        end
    elseif selected == 1 then
        if npc:buy(me) == DIALOG_RESULT_NEXT then
            goto ROUTINE_1
        end
    elseif selected == 2 then
        if npc:repair(me) == DIALOG_RESULT_NEXT then
            goto ROUTINE_1
        end
    elseif selected == 3 then
        if npc:repair_all(me) == DIALOG_RESULT_NEXT then
            goto ROUTINE_1
        end
    elseif selected == 4 then
        if npc:hold_money(me) == DIALOG_RESULT_NEXT then
           goto ROUTINE_1 
        end
    elseif selected == 5 then
        if npc:hold_item(me) == DIALOG_RESULT_NEXT then
           goto ROUTINE_1 
        end
    elseif selected == 6 then
        if npc:return_money(me) == DIALOG_RESULT_NEXT then
           goto ROUTINE_1 
        end
    elseif selected == 7 then
        if npc:return_item(me) == DIALOG_RESULT_NEXT then
           goto ROUTINE_1 
        end
    elseif selected == 8 then
        if npc:rename_weapon(me) == DIALOG_RESULT_NEXT then
           goto ROUTINE_1 
        end
    else

    end
end