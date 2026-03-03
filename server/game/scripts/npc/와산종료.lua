-- @note Trash: 1_백제\와산인던.txt "와산종료". Party warp + reward (십억경보상비 #server_exprate+5) + deletenpc. FB: single-player warp only; reward count fixed to 1; party warp and deletenpc omitted.

---@brief   NPC 와산종료: exit Wasan dungeon; warp to 와산성입구 and give reward. Party warp and deletenpc not implemented.
---@param[in]  me   The character.
---@param[in]  npc  The NPC entity.
function NPC_420(me, npc)
    local map = name2map("와산성입구")
    if map == nil then
        me:dialog(npc, "이동할 수 없습니다.", false, false)
        return
    end
    local x = 10 + math.random(1, 4)
    local y = 33
    me:mkitem("십억경보상비", 1)
    me:map(map, x, y)
end
