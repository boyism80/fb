-- 정지 (id 15). Add logic as needed.
local spell = require('lib.spell')

function ON_CAST_15(me, you, sp)
    if not you:is(OBJECT_TYPE.CHARACTER) then
        me:message('걸리지 않습니다.')
        return
    end

    if you:role() >= me:role() then
        me:message('걸리지 않습니다.')
        return
    end

    local mp = 80
    local sound = 70
    local effect = 1
    local buff_time = 20
    if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect}) then
        you:buff(sp, buff_time, me)
    end
end

function ON_BUFF_15(me, sp)
    me:freeze(true)
end

function ON_UNBUFF_15(me, sp)
    me:freeze(false)
end
