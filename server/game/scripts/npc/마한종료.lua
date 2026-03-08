-- @note Trash: 1_백제\마한인던.txt "마한종료". Mapid 6702/6704/6706/6708 -> 마한대기실1/2/3/4; party warp + reward (십억경보상비 #server_exprate/5) + deletenpc. FB: warp to 마한대기실1 (mapid-based exit omitted); reward count 1; party warp and deletenpc omitted.

---@brief   NPC 마한종료: exit Mahan dungeon; warp to waiting room and give reward. Party warp, mapid-based room, and deletenpc not implemented.
---@param[in]  me   The character.
---@param[in]  npc  The NPC entity.
function NPC_410(me, npc)
    local map = name2map("마한대기실1")
    if map == nil then
        me:dialog(npc, "이동할 수 없습니다.", false, false)
        return
    end
    me:mkitem("십억경보상비", 1)
    me:map(map, 7, 12)
end
