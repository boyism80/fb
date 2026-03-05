-- 의태 (id 14). Add logic as needed.
function ON_CAST_14(me, you, spell)
    local buff_time = 5
    if buff_cast(me, me, spell) then
        local appearance = you:appearance()
        me:mimic(appearance)
        me:buff(spell, buff_time, me)
    end
end

function ON_BUFF_14(me, spell)
    -- TODO: implement
end

function ON_UNBUFF_14(me, spell)
    me:mimic(nil)
end
