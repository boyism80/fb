function on_interact(me, npc)
::ROUTINE_1::
	local selected = npc:menu(me, '안녕하세요. 무엇을 도와드릴까요?', {'물건 사기', '물건 팔기', '물건 고치기', '모두 고치기', '메뉴3', '메뉴4', '메뉴5'})
	if selected == nil then
		return
	end

	if selected == 0 then
		if npc:sell(me) == DIALOG_RESULT_NEXT then
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
	else

	end
end