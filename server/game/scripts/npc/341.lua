-- npc: 토토입장
function NPC_341(me, npc)
    local sel, btn = me:list(npc, "몬스터복불복에 입장하실래요?", {"네.", "아니요."}, { prev = false })
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if sel == nil then
        return
    end

    if sel == 1 then
        local map = name2map("복불복몬스터토토")
        if map then
            me:map(map, math.random(20, 25), math.random(10, 20))
        end
    elseif sel == 2 then
        me:dialog(npc, "정답입니다.", { prev = false, next = true })
    end
end