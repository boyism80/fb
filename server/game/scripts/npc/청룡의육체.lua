-- @note Trash: 1_단군\일반.txt "청룡의육체". get_job(@sd)==4 only; else ".....zZZ". Else callfunc "ANIMAL_ITEM","청룡". Job 4 = 청룡; mapped to CLASS.POET. ANIMAL_ITEM not implemented.

---@brief   NPC 청룡의육체: only responds to Cheongryong (job 4) class; ANIMAL_ITEM (revival incense, spawn) omitted.
---@param[in]  me   The character.
---@param[in]  npc  The NPC entity.
function NPC_431(me, npc)
    if me:class() ~= CLASS.POET then
        me:dialog(npc, ".....zZZ", false, false)
        return
    end
    me:dialog(npc, "......신의 육체는 특별한 향료로 깨울 수 있으니...", false, false)
end
