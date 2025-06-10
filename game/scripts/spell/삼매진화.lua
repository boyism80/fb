-- 삼매진화 캐스팅
function ON_CAST_3044(me, you, spell)
    local damage = math.floor(me:mp() * 1.5)
    local mp = me:mp()
    local sound = 73
    local effect = 8
    spell_damage_near_target(me, you, spell, {damage,=damage mp=mp, sound=sound, effect=effect})
end