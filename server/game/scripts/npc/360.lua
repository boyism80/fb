-- npc: 장명지
function NPC_360(me, npc)
    local btn = me:dialog(npc, "안녕하세요? 저는 장명지입니다.", false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end

    local t = datetime()
    local month = (t and t.month) and t.month or 1
    if month < 1 or month > 12 then
        month = 1
    end
    me:dialog(npc, "지금은 " .. tostring(month) .. "월이네요. 이 달 이야기는 준비중입니다.", false, false)
end
