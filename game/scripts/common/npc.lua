function sale(me, npc, pursuit)
	local list = pursuit_sale(pursuit)
	local selected = npc:item(me, '제가 파는 물건들입니다. 그림도 있고, 옆에 가격도 함께 드리니 잘 생각하시고 골라주세요.', list)

	local item = name2item(selected)
	if item:attr(ITEM_ATTR_BUNDLE) then
		local count = npc:input(me, '몇개?')
	end
	npc:dialog(me, selected)
end