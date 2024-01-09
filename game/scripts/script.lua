-- script/script.lua

function handle_click(me, npc)
    npc:item(me, 'message', { name2item('동동주'), name2item('막걸리'), '도토리' = 300})
    npc:item(me, 'message', {'동동주', '막걸리', '도토리' = 300})

    local group = me:group()
    if group == nil then
        if npc:dialog(me, '그룹이 없습니다. QUIT을 누르세요', true, true) == DIALOG_RESULT_QUIT then
            npc:dialog(me, '그렇지')
        else
            npc:dialog(me, '말 존나 안들어쳐먹네')
        end
    else
        local message = '그룹원 현황\n'
        for _, member in pairs(group:members()) do
            message = message .. member:name() .. '\n'
        end
        npc:dialog(me, message)
    end
end