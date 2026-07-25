-- spell: 의태
-- 의태 (id 14). Add logic as needed.
local spell = require('lib.spell')

--     -- TODO: implement

return {
    ON_CAST = function(me, you, sp)
        local buff_time = 5
        if spell.buff_cast(me, me, sp) then
            local appearance = you:appearance()
            me:mimic(appearance)
            me:buff(sp, buff_time, me)
        end
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    ON_UNBUFF = function(me, sp)
        me:mimic(nil)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
