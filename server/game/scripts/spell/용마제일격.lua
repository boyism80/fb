-- 용마제일격: deal magic damage to the target. Trash: damage 2000~2500, effect 51, sound 8, mp 100.
function ON_CAST_21(me, you, spell)
    local damage = math.random(2000, 2500)
    local mp = 100
    local sound = 8
    local effect = 51
    spell_damage(me, you, spell, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_21(me, spell)
end

function ON_UNBUFF_21(me, spell)
end
