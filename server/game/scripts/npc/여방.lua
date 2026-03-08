-- @note Trash: 0_홍보관련\홍보.txt "여방". Eligibility by #goddess_1$..20 name arrays; reward 여신의방패상자1. No gv array in FB: show info message only.

---@brief   NPC 여방: promo box eligibility (trash used server arrays); FB shows info message only.
---@param[in] me   The character.
---@param[in] npc  The NPC entity.
function NPC_387(me, npc)
    me:dialog(npc, "추억서버를 위한 홍보를 하실 경우 '여방상자'라는 아이템을 받으실 수 있습니다.\n여방상자는 '여신의방패'를 낮은 확률로 얻을 수 있답니다..", true, false)
end
