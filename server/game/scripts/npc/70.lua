-- npc: 부여경비병

return {
    ON_CLICK = function(me, npc)
        me:dialog(npc, '여기는 부여왕이 계시는 곳입니다.', { prev = false, next = false })
    end
}
