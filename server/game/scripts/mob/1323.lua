-- mob: 주머니해골왕

return {
    on_mob_attack = function(me, you)
        me:chat('이대로 죽을 성 싶으냐!!', CHAT_TYPE.NORMAL, true)
    end
}
