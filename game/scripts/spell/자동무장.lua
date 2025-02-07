function on_cast(me, spell)
    if me:isbuff(table.unpack(relative_buff_name(spell:name()))) then
        return false
    end

    me:sound(8)
    me:effect(110)
    me:buff(spell, 180, me)
end

function on_buff(me, spell)
    me:buff_phydef(me:buff_phydef() -10)
end

function on_unbuff(me, spell)
    me:buff_phydef(me:buff_phydef() + 10)
end