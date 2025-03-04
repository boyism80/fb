-- 경수 캐스팅
function ON_CAST_2004(me, spell, name)
    local mobs = {
        ['토끼'] = name2mob('토끼'):look(),
        ['다람쥐'] = name2mob('다람쥐'):look(),
    }

    local mp = 30
    local effect = 3
    local sound = 25
    local buff_time = 60
    spell_disguise(me, mobs, name, spell, mp, sound, effect, buff_time)
end

-- 경수 버프 효과
function ON_BUFF_2004(me, spell)

end

-- 경수 버프 해제 효과
function ON_UNBUFF_2004(me, spell)
    me:disguise(nil)
end