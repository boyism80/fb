function on_cast(me, spell)
    local mp = 30
    local sound = 21
    local effect = 11
    local buff_time = 180
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time)
    end
end

function on_buff(me, spell)
    me:buff_str(me:buff_str()+3)
end

function on_unbuff(me, spell)
    me:buff_str(me:buff_str()-3)
end