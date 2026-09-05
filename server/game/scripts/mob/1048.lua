-- mob: 암흑왕

return {
    on_mob_attack = function(me, you)
        me:chat('내가 이정도로 죽을것 같으냐!!', CHAT_TYPE.NORMAL, true)
        return false
    end
}
