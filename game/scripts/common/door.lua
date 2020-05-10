function handle_door(session)
	local map = session:map()
	local door = map:door(session)
	if door == nil then
		return
	end

	local key = session:item('�Ķ�����')
	local locked = door:locked()
	if door:locked() and key == nil then
		session:message('���� ����ֽ��ϴ�.')
		return
	end

	local opened = door:toggle()
	if opened then
		session:message('���� �������ϴ�.')
	else
		session:message('���� �ݾҽ��ϴ�.')
	end
	door:update()
end