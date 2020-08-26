-- script/script.lua

DIALOG_RESULT_PREV = 0
DIALOG_RESULT_QUIT = 1
DIALOG_RESULT_NEXT = 2

function handle_click(session, npc)
	local group = session:group()
	if group == nil then
		npc:dialog(session, '�׷��� �����ϴ�.')
		npc:dialog(session, '�׷��� �����ϴ�.')
		npc:dialog(session, '�׷��� �����ϴ�.')
	else
		local message = '�׷�� ��Ȳ\n'
		for _, member in pairs(group:members()) do
			message = message .. member:name() .. '\n'
		end
		npc:dialog(session, message)
	end
end