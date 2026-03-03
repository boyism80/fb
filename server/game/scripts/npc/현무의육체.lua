-- @note Trash: 1_단군\일반.txt "현무의육체". get_job(@sd)==1 only; else ".....zZZ". Else callfunc "ANIMAL_ITEM","현무". Job 1 = 현무; mapped to CLASS.WARRIOR. ANIMAL_ITEM not implemented.

---@brief   NPC 현무의육체: only responds to Hyunmu (job 1) class; ANIMAL_ITEM (revival incense, spawn) omitted.
---@param[in]  me   The character.
---@param[in]  npc  The NPC entity.
function NPC_432(me, npc)
    if me:class() ~= CLASS.WARRIOR then
        me:dialog(npc, ".....zZZ", false, false)
        return
    end
    me:dialog(npc, "......신의 육체는 특별한 향료로 깨울 수 있으니...", false, false)
end
