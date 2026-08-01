-- spell: 정지
-- 정지 (id 15). Add logic as needed.
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
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
    end,

    on_buff = function(me, sp)
        me:freeze(true)
    end,

    on_unbuff = function(me, sp)
        me:freeze(false)
    end,

    -- on_concast = function(me, sp)
    -- end
}
