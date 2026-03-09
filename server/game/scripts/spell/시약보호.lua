function ON_CAST_17(me, you, spell)
    local mp = 30
    local sound = 8
    local effect = 110
    local buff_time = 180
    if buff_cast(me, you, spell, { mp = mp, sound = sound, effect = effect }) then
        you:buff(spell, buff_time, me)
    end
end

function ON_BUFF_17(me, spell)
    me:damage_derate(me:damage_derate() + 1000)
end

function ON_UNBUFF_17(me, spell)
    me:damage_derate(me:damage_derate() - 1000)
end
