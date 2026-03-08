-- @note Trash: 1_백두촌\일반.txt "초도". First dialog "......"; then if $백두산==0 "......" and end; if $백두산==1 long chain (천부연 intro, list). FB: no global $백두산; show intro "......" only; 백두산==1 branch can be added later via quest.

---@brief   NPC 초도: Cheonbuyeon intro; shows "......" (full $백두산==1 dialog chain omitted without quest/global).
---@param[in]  me   The character.
---@param[in]  npc  The NPC entity.
function NPC_604(me, npc)
    local button = me:dialog(npc, "......", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
