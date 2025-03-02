-- 금수 캐스팅
function ON_CAST_2002(me, spell, name)
    local mobs = {
        ['고양이'] = 32950,
        ['새끼돼지'] = name2mob('꼬마돼지'):look(),
        ['돼지'] = name2mob('돼지'):look(),
        ['강아지'] = name2mob('강아지'):look(),
        ['누렁이'] = name2mob('누렁이'):look()
    }

    local mp = 30
    local effect = 3
    local sound = 25
    local buff_time = 60
    spell_disguise(me, mobs, name, spell, mp, sound, effect, buff_time)
end

-- 금수 버프 효과
function ON_BUFF_2002(me, spell)
    
end

-- 금수 버프 해제 효과
function ON_UNBUFF_2002(me, spell)
    me:disguise(nil)
end