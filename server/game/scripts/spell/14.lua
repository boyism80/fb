-- spell: 의태
-- 의태 (id 14). Add logic as needed.
local spell = require('lib.spell')

function ON_CAST_14(me, you, sp)
    local buff_time = 5
    if spell.buff_cast(me, me, sp) then
        local appearance = you:appearance()
        me:mimic(appearance)
        me:buff(sp, buff_time, me)
    end
end

function ON_BUFF_14(me, sp)
    -- TODO: implement
end

function ON_UNBUFF_14(me, sp)
    me:mimic(nil)
end
