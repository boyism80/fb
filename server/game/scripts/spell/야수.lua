-- 야수 캐스팅
function ON_CAST_2006(me, spell, name)
    local mobs = {
        ['여우'] = name2mob('여우'):look(),
        ['늑대'] = name2mob('늑대'):look(),
    }

    local mp = 30
    local effect = 3
    local sound = 25
    local buff_time = 60
    spell_disguise(me, mobs, name, spell, {mp=mp, sound=sound, effect=effect, buff_time=buff_time})
end

-- 야수 버프 효과
function ON_BUFF_2006(me, spell)

end

-- 야수 버프 해제 효과
function ON_UNBUFF_2006(me, spell)
    me:mimic(nil)
end