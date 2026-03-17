function ON_CAST_39(me, you, spell)
    local mp = 40
    local sound = 40
    local effect = 53
    local buff_time = 6
    if debuff_cast(me, you, spell, { mp = mp, sound = sound, effect = effect }) then
        you:buff(spell, buff_time, me)
    end
end

function ON_BUFF_39(me, spell)
    me:buff_phydef(me:buff_phydef() + 31)
end

function ON_UNBUFF_39(me, spell)
    me:buff_phydef(me:buff_phydef() - 31)
end
