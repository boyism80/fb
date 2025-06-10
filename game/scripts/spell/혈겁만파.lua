-- 혈겁만파 캐스팅
function ON_CAST_1021(me, spell)
    local damage = (me:hp()*3)//4
    local hp = me:hp()//2
    local mp = 600
    local sound = 507
    local effect = 162

    spell_damage_area(me, me:nears(OBJECT_TYPE_LIFE, 6, 5), spell, {damage=damage, hp=hp, mp=mp, sound=sound, effect=effect})
end