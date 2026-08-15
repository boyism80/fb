-- mob: 구미호

return {
    on_mob_attack = function(me, you)
        local lines = {
            '구미호: 나의 거처에서 썩 꺼져라!!',
            '구미호: 정신이 혼미해질걸..??',
            '구미호: 고얀인간!',
            '구미호: 예가 어디라고!!',
        }
        me:chat(lines[math.random(#lines)], CHAT_TYPE.NORMAL, false)
    end
}
