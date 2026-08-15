-- mob: 유성지

return {
    on_mob_attack = function(me, you)
        me:chat('나 유성지가 쉽게 당할것 같으냐!!?', CHAT_TYPE.NORMAL, true)
        return false
    end,

    on_mob_die = function(me)
        me:chat('흐아압 !!', CHAT_TYPE.NORMAL, true)
    end
}
