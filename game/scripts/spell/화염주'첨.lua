-- 화염주'첨 캐스팅
function ON_CAST_3022(me, spell)
	local damage = 300
    local mp = 180
    local sound = 40
    local effect = 45
    spell_damage_near(me, spell, damage, mp, sound, effect)
end