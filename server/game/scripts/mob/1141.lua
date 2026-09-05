-- mob: 해골왕

return {
    on_mob_attack = function(me, you)
        me:chat('이대로 죽을 성 싶으냐!!', CHAT_TYPE.NORMAL, true)
        return false
    end,

    on_mob_die = function(me)
        local lines = {
        '크크크크....',
        '인간따위가 감히...',
    }
        me:chat(lines[math.random(#lines)], CHAT_TYPE.NORMAL, true)
    end
}
