-- npc: 백남신
function NPC_358(me, npc)
    local d = me:dialog(npc, "안녕하세요? 저는 백남신입니다.", false, true)
    if d == DIALOG_RESULT.QUIT then
        return
    end

    local t = datetime()
    local month = (t and t.month) and t.month or 1
    if month < 1 or month > 12 then
        month = 1
    end
    me:dialog(npc, "지금은 " .. tostring(month) .. "월이네요. 이 달 이야기는 준비중입니다.", false, false)
end
