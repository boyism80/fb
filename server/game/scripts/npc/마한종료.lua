
function NPC_410(me, npc)
    local map = name2map("마한대기실1")
    if map == nil then
        me:dialog(npc, "이동할 수 없습니다.", false, false)
        return
    end
    me:mkitem("십억경보상비", 1)
    me:map(map, 7, 12)
end
