-- @note Trash: 1_백제\칠중하인던.txt "칠중하종료". Party check (no party -> message); reward by party size; warp party to 칠중하입구; deletenpc. FB: single-player warp + reward 1; party/deletenpc omitted.

---@brief   NPC 칠중하종료: exit Chiljungha dungeon; warp to 칠중하입구 and give reward. Party warp and deletenpc not implemented.
---@param[in]  me   The character.
---@param[in]  npc  The NPC entity.
function NPC_546(me, npc)
    local map = name2map("칠중하입구")
    if map == nil then
        me:dialog(npc, "이동할 수 없습니다.", false, false)
        return
    end
    local x = math.random(4, 30)
    local y = math.random(9, 12)
    me:mkitem("십억경보상비", 1)
    me:map(map, x, y)
end
