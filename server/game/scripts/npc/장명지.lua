-- @note Trash: 1_세시마을\삼짇집.txt "장명지". Intro dialog + gettime(4,0) 1..12 branches; trash branches empty, placeholder by month.

---@brief   NPC 장명지: intro + month-based placeholder (trash had empty branches per month).
---@param[in] me   The character.
---@param[in] npc  The NPC entity.
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
