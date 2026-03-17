function ON_CAST_18(me, you, spell)
    local mp = 30
    local sound = 8
    local effect = 110
    local buff_time = 180
    if buff_cast(me, you, spell, { mp = mp, sound = sound, effect = effect }) then
        you:buff(spell, buff_time, me)
    end
end

function ON_BUFF_18(me, spell)
    me:buff_phydef(me:buff_phydef() - 10)
end

function ON_UNBUFF_18(me, spell)
    me:buff_phydef(me:buff_phydef() + 10)
end
