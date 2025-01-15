function on_cast(me, spell, name)
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

function on_uncast(me, spell)
    unbuff(me, spell)
    me:disguise(nil)
end