function on_cast(me, spell)
    local buff_time = 180
    if me:isbuff(table.unpack(relative_buff_name(spell:name()))) then
        return false
    end

    me:buff(spell, buff_time)
end

function on_buff(me, spell)
    me:state(STATE_HALF_CLOACK)
end

function on_unbuff(me, spell)
    me:state(STATE_NORMAL)

end