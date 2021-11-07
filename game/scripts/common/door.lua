function on_door(session)
	local map = session:map()
	local door = map:door(session)
	if door == nil then
		return
	end

	local key = session:item('파란열쇠')
	local locked = door:locked()
	if door:locked() and key == nil then
		session:message('문이 잠겨있습니다.')
		return
	end

	local opened = door:toggle()
	if opened then
		session:message('문을 열었습니다.')
	else
		session:message('문을 닫았습니다.')
	end
	door:update()
end