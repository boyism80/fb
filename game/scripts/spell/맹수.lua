-- 맹수 캐스팅
function ON_CAST_2007(me, spell, name)
    local mobs = {
        ['곰'] = name2mob('곰'):look(),
        ['호랑이'] = name2mob('호랑이'):look(),
    }

    local mp = 30
    local effect = 3
    local sound = 25
    local buff_time = 60
    spell_disguise(me, mobs, name, spell, mp, sound, effect, buff_time)
end

-- 맹수 버프 효과
function ON_BUFF_2007(me, spell)

end

-- 맹수 버프 해제 효과
function ON_UNBUFF_2007(me, spell)
    me:disguise(nil)
end