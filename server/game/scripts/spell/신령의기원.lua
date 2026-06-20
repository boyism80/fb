-- 신령의기원 캐스팅
local spell = require('lib.spell')

function ON_CAST_4038(me, you, sp)
    local map = me:map()
    if map == nil then
        return
    end

    local mp = 1000
    local sound = 97
    local effect = 63
    if not spell.cast(me, you, sp, {mp=mp, sound=sound, effect=effect}) then
        return
    end
    
    if you:is(OBJECT_TYPE.LIFE) then
        you:heal(10000)
    end
end