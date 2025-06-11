-- 신령의기원'첨 캐스팅
function ON_CAST_4039(me, spell)
    local mp = 1000
    local sound = 97
    local effect = 63
    local hp = 10000
    if not spell_heal_group(me, spell, {hp=hp, mp=mp, sound=sound, effect=effect}) then
        return
    end
end