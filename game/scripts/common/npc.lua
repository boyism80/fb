function sell(me, npc, pursuit)
	local list = pursuit_sell(pursuit)
::ROUTINE1::
	local selected = npc:item(me, '제가 파는 물건들입니다. 그림도 있고, 옆에 가격도 함께 드리니 잘 생각하시고 골라주세요.', list)
	if selected == nil then
		return DIALOG_RESULT_NEXT
	end

	local item = name2item(selected)
	local count = 1
	if item:attr(ITEM_ATTR_BUNDLE) then
		count = npc:input(me, '몇개나 사시겠어요?')
		if count == nil then
			goto ROUTINE1
		end

		count = tonumber(count)
		if count == nil or count <= 0 then
			return npc:dialog(me, '갯수가 올바르지 않습니다.', false, true)
		end
	end

	local price = list[selected] * count
	local money = me:money()
	if price > money then
		return npc:dialog(me, '돈이 모자랍니다.', false, true)
	end

	local exist_count = 0
	local exist = me:item(selected)
	if exist ~= nil then
		exist_count = exist:count()
	end

	if exist_count + count > item:capacity() then
		return npc:dialog(me, '더 이상 가질 수 없습니다.', false, true)
	end
	
	if me:mkitem(selected, count) == nil then
		return npc:dialog(me, '공간이 부족합니다.', false, true)
	else
		me:money(money - price)
		local message = string.format('%s %d개를 %d전에 팔았습니다.', selected, count, price)
		return npc:dialog(me, message, false, true)
	end
end

function repairable_slots(me)
	local items = me:items()
	local slots = {}
	for slot, item in ipair(items) do
		local model = item:model()
		if model:attr(ITEM_ATTR_EQUIPMENT) then
			local current = item:durability()
			local max = model:durability()
			if current < max then
				slots[slot] = item
			end
		end
	end

	return slots
end

function repair(me, npc)
	local items = repairable_slots(me)
	if #items == 0 then
		return npc:dialog(me, '고칠 물건이 없는데요', false, true)
	end

	local slots = {}
	for slot, item in pair(slots) do
		table.insert(slots, slot)
	end

	return npc:slot(me, '무엇을 고치시겠습니까?', slots)
end


function repair_all(me, npc)
	local items = repairable_slots(me)
	if #items == 0 then
		return npc:dialog(me, '고칠 물건이 없는데요', false, true)
	end

	local slots = {}
	local price = 0
	for slot, item in pair(items) do
		local model = item:model()
		local current = item:durability()
		local max = model:durability()
		price = price + (max - current)
	end

	local selected = npc:dialog(me, string.format('모두 고치는데 %d전이 필요합니다. 고치시겠습니까?', price), {'네', '아니오'})
	if selected == 0 then
		local money = me:money()
		if money < price then
			return npc:dialog(me, '돈이 모자랍니다.')
		else
			for _, slot in ipair(slots) do
				local item = items[slot]
				local model = item:model()
				item:durability(model:durability())
			end

			me:money(money - price)
			return npc:dialog(me, '모두 고쳤습니다.')
		end
	else
		return DIALOG_RESULT_NEXT
	end
end