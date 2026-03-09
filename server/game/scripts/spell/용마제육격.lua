function ON_CAST_26(me, you, spell)
    local damage = math.random(78000, 80000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell_damage(me, you, spell, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_26(me, spell)
end

function ON_UNBUFF_26(me, spell)
end
