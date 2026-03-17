function ON_CAST_38(me, you, spell)
    local damage = math.random(250000, 280000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell_damage(me, you, spell, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_38(me, spell)
end

function ON_UNBUFF_38(me, spell)
end
