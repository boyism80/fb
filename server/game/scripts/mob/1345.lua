-- mob: 계피귀신
local LINES = {
    "계피귀신: 으아앙~! ",
    "계피귀신: 때리지 마세요~!",
    "계피귀신: 계피 가져가면 아파요 으아앙~!",
    "계피귀신: 엉엉~! 살려주세요~!",
    "계피귀신: 왜 그러는거에요....!",
}

return {
    on_mob_attack = function(me, you)
        if me == nil then
            return
        end
        if math.random(1, 3) == 1 then
            me:chat(LINES[math.random(1, #LINES)])
        end
    end,

    on_mob_die = function(me)
        if me == nil then
            return
        end
        me:chat("계피귀신: 으아아앙~!!")
    end,
}
