function handle_active(session, item)
	local map = session:map()
	local door = map:door(session)
	if door == nil then
		print('door is nil')
		return
	end

	local locked = door:lock(not door:locked())
	if locked then
		session:message('���� ��ɽ��ϴ�.')
	else
		session:message('���� �������ϴ�.')
	end
end