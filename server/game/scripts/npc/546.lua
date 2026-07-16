-- npc: 칠중하종료
function NPC_546(me, npc)
    local map = name2map("칠중하입구")
    if map == nil then
        me:dialog(npc, "이동할 수 없습니다.", { prev = false, next = false })
        return
    end
    local x = math.random(4, 30)
    local y = math.random(9, 12)
    me:mkitem("십억경보상비", 1)
    me:map(map, x, y)
end
