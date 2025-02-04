function on_cast(me, spell)
    local buff_time = 180
    if me:isbuff(table.unpack(relative_buff_name(spell:name()))) then
        return false
    end

    me:state(STATE_HALF_CLOACK)
    me:buff(spell, buff_time)
end

function on_uncast(me, spell)
    me:state(STATE_NORMAL)
    unbuff(me, spell)
end