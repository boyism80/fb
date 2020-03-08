-- script/script.lua

DIALOG_RESULT_PREV = 0
DIALOG_RESULT_QUIT = 1
DIALOG_RESULT_NEXT = 2

function handle_click(session, npc)
    local x, y = session:position()
	local action = npc:input_dialog_ext(session, string.format('안녕하세요 %s님. \n\n현재 위치는 %s, %s입니다.', session:name(), x, y), "갓승현님을 존경하는 저의 이름은", "입니다.")

	if type(action) == 'string' then
		if npc:dialog(session, string.format('"%s"라닛?', action), false, true) == DIALOG_RESULT_NEXT then
			npc:dialog(session, "next : 갓승현님이 나아가실 앞으로의 찬란한 인생을 뜻하는 단어")

			local selected = npc:menu_dialog(session, '골라골라 골라판!', {"hi 1", "hi 2", "hi 3", "hi 4", "hi 5", "hi 1", "hi 2", "hi 3", "hi 4", "hi 5"})
			npc:dialog(session, string.format('selected : %d', selected))
		end

	elseif action == DIALOG_RESULT_QUIT then
		message = npc:input_dialog(session, "아무거나 입력해보세요")
		npc:dialog(session, string.format('입력한 메시지 : %s', message))
	end
end