-- mob: 용겸

return {
    on_mob_attack = function(me, you)
        return false
    end,

    on_mob_die = function(me)
        local lines = {
        '콰롸롸롸~',
        '크오오~~ 인간이 감히 나를~!!',
    }
        me:chat(lines[math.random(#lines)], CHAT_TYPE.NORMAL, true)
    end
}
