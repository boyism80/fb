-- mob: 진백호왕

return {
    on_mob_attack = function(me, you)
        return false
    end,

    on_mob_die = function(me)
        me:chat('크롸롸!!', CHAT_TYPE.NORMAL, true)
    end
}
