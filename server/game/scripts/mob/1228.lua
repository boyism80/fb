-- mob: 망퉁산

return {
    on_mob_attack = function(me, you)
        me:chat('우리에게 양식을 다오', CHAT_TYPE.NORMAL, true)
    end
}
