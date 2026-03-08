-- @note Trash: 1_세시마을\중구집.txt "백남신". Intro dialog + month-based branches (gettime(4,0) 1..12); trash branches empty, placeholder per month.

---@brief NPC 백남신: intro + month-based message (placeholder; trash had empty branches per month).
---@param[in] me  The character.
---@param[in] npc The NPC entity.
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
