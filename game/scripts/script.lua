-- script/script.lua

DIALOG_RESULT_PREV = 0
DIALOG_RESULT_QUIT = 1
DIALOG_RESULT_NEXT = 2

function handle_click(session, npc)
	local group = session:group()
	if group == nil then
		npc:dialog(session, '그룹이 없습니다.')
		npc:dialog(session, '그룹이 없습니다.')
		npc:dialog(session, '그룹이 없습니다.')
	else
		local message = '그룹원 현황\n'
		for _, member in pairs(group:members()) do
			message = message .. member:name() .. '\n'
		end
		npc:dialog(session, message)
	end
end