-- 삼성백열장: deal magic damage to the selected target.
function ON_CAST_105(me, you, spell)
    local damage = 796
    local mp = 420
    local sound = 8
    local effect = 86
    spell_damage(me, you, spell, { damage = damage, mp = mp, sound = sound, effect = effect })
end
