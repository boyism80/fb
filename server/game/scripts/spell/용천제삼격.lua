-- 용천제삼격: deal magic damage to the target. Trash: damage 18000~20000, effect 51, sound 8, mp 100.
function ON_CAST_32(me, you, spell)
    local damage = math.random(18000, 20000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell_damage(me, you, spell, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_32(me, spell)
end

function ON_UNBUFF_32(me, spell)
end
