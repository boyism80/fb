-- 오성백열장: deal magic damage to the selected target.
function ON_CAST_107(me, you, spell)
    local damage = 1380
    local mp = 700
    local sound = 8
    local effect = 86
    spell_damage(me, you, spell, { damage = damage, mp = mp, sound = sound, effect = effect })
end
