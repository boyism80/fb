-- script/script.lua

function handle_click(session, npc)
    local group = session:group()
    if group == nil then
        if npc:dialog(session, '그룹이 없습니다. QUIT을 누르세요', true, true) == DIALOG_RESULT_QUIT then
            npc:dialog(session, '그렇지')
        else
            npc:dialog(session, '말 존나 안들어쳐먹네')
        end
    else
        local message = '그룹원 현황\n'
        for _, member in pairs(group:members()) do
            message = message .. member:name() .. '\n'
        end
        npc:dialog(session, message)
    end
end