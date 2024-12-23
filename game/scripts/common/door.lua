function on_door(ch)
	local map = ch:map()
	local door = map:door(ch)
	if door == nil then
		return
	end

	local key = ch:item('파란열쇠')
	local locked = door:locked()
	if door:locked() and key == nil then
		ch:message('문이 잠겨있습니다.')
		return
	end

	local opened = door:toggle()
	if opened then
		ch:message('문을 열었습니다.')
	else
		ch:message('문을 닫았습니다.')
	end
end