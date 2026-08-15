-- mob: 코가닌자'진

return {
    on_mob_attack = function(me, you)
        me:chat('코가닌자\'술!!', CHAT_TYPE.NORMAL, true)
    end
}
