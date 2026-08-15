-- mob: 운랑

return {
    on_mob_attack = function(me, you)
        me:chat('폭뢰부활!!', CHAT_TYPE.NORMAL, true)
        return false
    end
}
