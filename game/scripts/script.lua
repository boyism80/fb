-- script/script.lua

DIALOG_RESULT_PREV = 0
DIALOG_RESULT_QUIT = 1
DIALOG_RESULT_NEXT = 2

function handle_click(session, npc)
	local x, y = session:position()
	local action = npc:input_dialog(session, string.format('�ȳ��ϼ��� %s��. \n\n���� ��ġ�� %s, %s�Դϴ�.', session:name(), x, y), "���������� �����ϴ� ���� �̸���", "�Դϴ�.")

	if type(action) == 'string' then
		npc:chat(action)
		if npc:dialog(session, string.format('"%s"���?', action), false, true) == DIALOG_RESULT_NEXT then
			npc:dialog(session, "next : ���������� ���ư��� �������� ������ �λ��� ���ϴ� �ܾ�")

			local selected = npc:menu_dialog(session, '����� �����!', {"hi 1", "hi 2", "hi 3", "hi 4", "hi 5", "hi 1", "hi 2", "hi 3", "hi 4", "hi 5"})
			npc:dialog(session, string.format('selected : %d', selected))
		end

	elseif action == DIALOG_RESULT_QUIT then
		timer(10)
		message = npc:input_dialog(session, "�ƹ��ų� �Է��غ�����")
		npc:dialog(session, string.format('�Է��� �޽��� : %s', message))
	end

	session:action(6)
	session:buff('����.', 180)
	session:effect(110)
	session:sound(8)
	session:message('������ �ܿ����ϴ�.')
end