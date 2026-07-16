-- npc: 산소소탕병사
function NPC_210(me, npc)
    local r = math.random(1, 3)
    if r == 1 then
        me:dialog(npc, '여긴 어쩐일이요?', { prev = false, next = true })
    elseif r == 2 then
        me:dialog(npc, '우리와 같이 산소소탕을 하러 오신건가?', { prev = false, next = true })
    else
        me:dialog(npc, '에헴.. 내가 산소들을 몽땅 소탕해주지!', { prev = false, next = true })
    end
end
