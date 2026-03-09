function ON_CAST_30(me, you, spell)
    local damage = math.random(2000, 2500)
    local mp = 100
    local sound = 8
    local effect = 51
    spell_damage(me, you, spell, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_30(me, spell)
end

function ON_UNBUFF_30(me, spell)
end
