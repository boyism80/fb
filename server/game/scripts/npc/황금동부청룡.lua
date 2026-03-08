-- @note Trash: 1_황금성\사방신수.txt "황금동부청룡". Single dialog only (defeat soldiers first).

---@brief   NPC 황금동부청룡: challenges player to defeat soldiers first.
---@param[in]  me   The character.
---@param[in]  npc  The NPC entity.
function NPC_484(me, npc)
    local button = me:dialog(npc, "내게 도전하고 싶거든, 먼저 병사들을 모두 물리치고 오너라.", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
