-- mob: 파괴왕

return {
    on_mob_attack = function(me, you)
        return false
    end,

    on_mob_die = function(me)
        local lines = {
        '어리석은 인간들이여...',
        '파괴왕: 크으윽..',
    }
        me:chat(lines[math.random(#lines)], CHAT_TYPE.NORMAL, false)
    end
}
