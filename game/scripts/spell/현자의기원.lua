-- callfunc "TARGETHEALMAGIC", 1000, 5000, 168, 503, "현자의기원"
-- 현자의기원 캐스팅
function ON_CAST_3047(me, you, spell)
    local hp = 5000
    local mp = 1000
    local sound = 503
    local effect = 168
    spell_heal(me, you, spell, hp, mp, sound, effect)
end