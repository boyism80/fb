-- @note Trash: 1_국내성\국내놀이방.txt "빙돌삼". Placeholder message only (feature not implemented).

---@brief   NPC 빙돌삼: placeholder for future dice/game feature.
---@param[in]  me   The character.
---@param[in]  npc  The NPC entity.
function NPC_299(me, npc)
    local button = me:dialog(npc, "준비중입니다. 해당 기능 사용을 원하시면 홈페이지에 문의해 주세요.", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
