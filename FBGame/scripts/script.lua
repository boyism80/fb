-- script/script.lua

DIALOG_RESULT_PREV = 0
DIALOG_RESULT_QUIT = 1
DIALOG_RESULT_NEXT = 2

function handle_click(session, npc)
    local x, y = session:position()
	local action = npc:dialog(session, string.format('Hi %s. \n\nYour position is %s, %s', session:name(), x, y), false, true)
	if action == DIALOG_RESULT_NEXT then
		if npc:dialog(session, "NEXT¶ó´Ö??", false, true) == DIALOG_RESULT_NEXT then
			npc:dialog(session, "ÇÑ¹ø ´õ NEXT´Â ³Ê¹«ÇÏÀÝ¾Æ ±×¸¸ÇØ")
		end

	elseif action == DIALOG_RESULT_QUIT then
		if npc:dialog(session, "QUIT¶ó´Ö??") == DIALOG_RESULT_QUIT then
			npc:dialog(session, "¿¬¼Ó µÎ¹ø QUIT ½Ã¹ú·³ ±Â")
		end
	end
end