-- 공력주입 캐스팅
local spell = require('lib.spell')

function ON_CAST_4010(me, you, sp)
    if not you:is(OBJECT_TYPE.LIFE) then
        return me:message('걸리지 않습니다.')
    end

    local mp = me:mp()
    local sound = 80
    local effect = 18
    if spell.cast(me, you, sp, {mp=mp, sound=sound, effect=effect}) then
        you:mp(math.min(you:maxmp(), you:mp() + mp))
    end
end