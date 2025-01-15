function on_cast(me, spell)
    local mp = 30
    local sound = 21
    local effect = 11
    local buff_time = 5
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff_str(me:buff_str()+3)
        me:buff(spell, buff_time)
    end
end

function on_uncast(me, spell)
    me:buff_str(me:buff_str()-3)
    unbuff(me, spell)
end