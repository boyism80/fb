-- 용천제오격: deal magic damage to the target. Trash: damage 48000~50000, effect 51, sound 8, mp 100.
function ON_CAST_34(me, you, spell)
    local damage = math.random(48000, 50000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell_damage(me, you, spell, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_34(me, spell)
end

function ON_UNBUFF_34(me, spell)
end
