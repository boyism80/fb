-- spell: 의태
-- 의태 (id 14). Add logic as needed.
local spell = require('lib.spell')

--     -- TODO: implement

return {
    on_cast = function(me, you, sp)
        local buff_time = 5
        if spell.buff_cast(me, me, sp) then
            local appearance = you:appearance()
            me:mimic(appearance)
            me:buff(sp, buff_time, me)
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    on_unbuff = function(me, sp)
        me:mimic(nil)
    end,

    -- on_concast = function(me, sp)
    -- end
}
