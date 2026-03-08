-- 용천제팔격: deal magic damage to the target. Trash: damage 170000~200000, effect 51, sound 8, mp 100.
function ON_CAST_37(me, you, spell)
    local damage = math.random(170000, 200000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell_damage(me, you, spell, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_37(me, spell)
end

function ON_UNBUFF_37(me, spell)
end
