function NPC_157(me, npc)
::ROUTINE_CHANGE_ARMOR_COLOR_0::
    local index, button = npc:list(me, '안녕하세요. 어떻게 오셨나요?', {'무한 복장', '일반 복장'})
    if index == nil then
        return
    end

    if index == 0 then
        local price = 1000
        index = npc:list(me, string.format('무한에 참여하시려면 염색비 %d전을 내시고 무한 복장을 하셔야 합니다. %d전을 내시겠습니까?', price, price), {'예', '아니오'})
        if index == nil then
            return
        end

        if index ~= 0 then
            return
        end

        index = npc:list(me, '어떤 팀으로 참가하시겠습니까?', {'북팀 - 검은색', '남팀 - 붉은색', '동팀 - 파란색', '서팀 - 하얀색'})
        if index == nil then
            return
        end

        if index == 0 then
            me:armor_color(10);
        elseif index == 1 then
            me:armor_color(31);
        elseif index == 2 then
            me:armor_color(17);
        elseif index == 3 then
            me:armor_color(11);
        else
        end

        local money = me:money()
        if money < price then
            npc:dialog(me, '돈이 모자랍니다.')
            return
        end

::ROUTINE_CHANGE_ARMOR_COLOR_1::
        me:money(money - price)
        if npc:dialog(me, string.format('염색비로 %d전을 받았습니다.', price), false, true) == DIALOG_RESULT_QUIT then
            return
        end

        button = npc:dialog(me, '그럼 언제나 행운이 함께 하시기를 빌겠습니다.', true, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        if button == DIALOG_RESULT_PREV then
            goto ROUTINE_CHANGE_ARMOR_COLOR_1
        end

        goto ROUTINE_CHANGE_ARMOR_COLOR_0

    elseif index == 1 then
    	index = npc:list(me, '일반 복장으로 바꿔드릴까요?', {'예', '아니오'})
    	if index == nil then
    		return
    	end

    	if index ~= 0 then
    		return
    	end

    	me:armor_color(nil)
    	button = npc:dialog(me, '일반 복장으로 바꿔드렸습니다.', false, true)
    	if button == DIALOG_RESULT_QUIT then
    		return
    	end
    	
    	goto ROUTINE_CHANGE_ARMOR_COLOR_0
    else
    end
end