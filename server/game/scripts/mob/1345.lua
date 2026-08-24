-- mob: 계피귀신

local LINES = {
    "으아앙~!",
    "때리지 마세요~!",
    "계피 가져가면 아파요 으아앙~!",
    "엉엉~! 살려주세요~!",
    "왜 그러는거에요....!",
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
        me:chat("으아아앙~!!")
    end,

    -- on_mob_action = function(me, you)
    -- end,

    -- on_mob_kill = function(me, mobs)
    -- end,

    -- on_mob_spell_hit = function(me, you, spell)
    -- end,
}
