-- 백호의희원'첨 캐스팅
function ON_CAST_4040(me, spell)
    local hp = me:mp()*2
    local mp = me:mp()//2
    local sound = 95
    local effect = 64
    if not spell_heal_group(me, spell, {hp=hp, mp=mp, sound=sound, effect=effect}) then
        return
    end
end