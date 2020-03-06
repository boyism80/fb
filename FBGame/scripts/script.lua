-- script/script.lua

DIALOG_RESULT_PREV = 0
DIALOG_RESULT_QUIT = 1
DIALOG_RESULT_NEXT = 2

function handle_click(session, npc)
    local x, y = session:position()
	local action = npc:dialog(session, string.format('Hi %s. \n\nYour position is %s, %s', session:name(), x, y), false, true)
	if action == DIALOG_RESULT_NEXT then
		if npc:dialog(session, "NEXT���??", false, true) == DIALOG_RESULT_NEXT then
			npc:dialog(session, "�ѹ� �� NEXT�� �ʹ����ݾ� �׸���")
		end

	elseif action == DIALOG_RESULT_QUIT then
		if npc:dialog(session, "QUIT���??") == DIALOG_RESULT_QUIT then
			npc:dialog(session, "���� �ι� QUIT �ù��� ��")
		end
	end
end