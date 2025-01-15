function on_cast(me, spell, name)
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

function on_uncast(me, spell)
    unbuff(me, spell)
    me:disguise(nil)
end