-- mob: 화룡

return {

    on_mob_die = function(me)
        me:chat('크롸롸!!', CHAT_TYPE.NORMAL, true)
    end
}
