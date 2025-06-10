-- 포효검황 캐스팅
function ON_CAST_1020(me, spell)
    local damage = (me:hp()*11)//10
    local hp = me:hp()//2
    local mp = 600
    local sound = 101
    local effect = 85

    spell_damage_area(me, me:nears(OBJECT_TYPE_LIFE, 8, 7), spell, {damage=damage, hp=hp, mp=mp, sound=sound, effect=effect})
end