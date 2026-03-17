function ON_CAST_22(me, you, spell)
    local damage = math.random(8000, 10000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell_damage(me, you, spell, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_22(me, spell)
end

function ON_UNBUFF_22(me, spell)
end
