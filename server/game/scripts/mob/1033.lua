-- mob: 도삭산청의태자

return {
    on_mob_attack = function(me, you)
        local lines = {
            '여의주의 힘을 받은 용이여..',
            '그대 이름은 청룡일지다...',
            '나 청의태자의 이름으로 말하노니...',
            '네 포효를 적에게 발산하라!!',
            '청룡마령참~~!!',
        }
        me:chat(lines[math.random(#lines)], CHAT_TYPE.NORMAL, true)
        return false
    end
}
