-- 공력주입 캐스팅
function ON_CAST_4010(me, you, spell)
    if not you:is(OBJECT_TYPE_LIFE) then
        return me:message('걸리지 않습니다.')
    end

    local mp = me:mp()
    local sound = 80
    local effect = 18
    if spell_cast(me, you, spell, {mp=mp, sound=sound, effect=effect}) then
        you:mp(math.min(you:maxmp(), you:mp() + mp))
    end
end