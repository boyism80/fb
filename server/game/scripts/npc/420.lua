-- npc: 와산종료
function NPC_420(me, npc)
    local map = name2map("와산성입구")
    if map == nil then
        me:dialog(npc, "이동할 수 없습니다.", { prev = false, next = false })
        return
    end
    local x = 10 + math.random(1, 4)
    local y = 33
    me:mkitem("십억경보상비", 1)
    me:map(map, x, y)
end
