-- @note Trash: 1_단군\일반.txt "주작의육체". get_job(@sd)==3 only; else ".....zZZ". Else callfunc "ANIMAL_ITEM","주작". Job 3 = 주작; mapped to CLASS.MAGE. ANIMAL_ITEM not implemented.

---@brief   NPC 주작의육체: only responds to Jujak (job 3) class; ANIMAL_ITEM (revival incense, spawn) omitted.
---@param[in]  me   The character.
---@param[in]  npc  The NPC entity.
function NPC_430(me, npc)
    if me:class() ~= CLASS.MAGE then
        me:dialog(npc, ".....zZZ", false, false)
        return
    end
    me:dialog(npc, "......신의 육체는 특별한 향료로 깨울 수 있으니...", false, false)
end
