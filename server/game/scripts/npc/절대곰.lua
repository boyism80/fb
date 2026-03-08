-- @note Trash: 0_곰의계곡\후원메뉴.txt "절대곰". NPC says "zZz...쿨쿨..." (sleeping). No interaction beyond that.

---@brief   NPC 절대곰: sleeping bear; displays a single message.
---@param[in]  me   The character.
---@param[in]  npc  The NPC entity.
function NPC_459(me, npc)
    local button = me:dialog(npc, "zZz...쿨쿨...", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
