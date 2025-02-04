function on_cast(me, spell)
    if me:isbuff(table.unpack(relative_buff_name(spell:name()))) then
        return false
    end

    me:sound(8)
    me:effect(110)
    me:damage_derate(me:damage_derate() + 1000)
    me:buff(spell, 180, me)
end

function on_uncast(me, spell)
    me:damage_derate(me:damage_derate() - 1000)
    unbuff(me, spell)
end