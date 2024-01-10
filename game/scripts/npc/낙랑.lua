function on_interact(me, npc)
	local selected = npc:menu(me, '안녕하세요. 무엇을 도와드릴까요?', {'물건 사기', '물건 팔기', '메뉴1', '메뉴2', '메뉴3', '메뉴4', '메뉴5'})
	if selected == 0 then
		local pursuit = 1
		npc:sale(me, pursuit)
	end

	me:chat('success')
end