-- @note Trash: 1_단군\일반.txt "백호의육체". get_job(@sd)==2 only; else ".....zZZ". Else callfunc "ANIMAL_ITEM","백호" (grade5th, 부활의향료, mobspawn). Job 2 = 백호; mapped to CLASS.ROGUE. ANIMAL_ITEM not implemented.

---@brief   NPC 백호의육체: only responds to Baekho (job 2) class; ANIMAL_ITEM (revival incense, spawn) omitted.
---@param[in]  me   The character.
---@param[in]  npc  The NPC entity.
function NPC_429(me, npc)
    -- Trash: get_job(@sd) != 2 -> ".....zZZ". Job 2 = 백호; FB class mapping: ROGUE.
    if me:class() ~= CLASS.ROGUE then
        me:dialog(npc, ".....zZZ", false, false)
        return
    end
    -- ANIMAL_ITEM: $grade5th, 부활의향료, list, mobspawn not implemented.
    me:dialog(npc, "......신의 육체는 특별한 향료로 깨울 수 있으니...", false, false)
end
