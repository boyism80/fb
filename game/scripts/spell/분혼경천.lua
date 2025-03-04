-- 분혼경천 캐스팅
function ON_CAST_2023(me, spell)
    local damage = (me:hp()*3)//4
    local hp = me:hp()//2
    local mp = 200
    local sound = 508
    local effect = 160

    spell_damage_area(me, me:nears(OBJECT_TYPE_LIFE, 7, 6), spell, damage, hp, mp, sound, effect)
end