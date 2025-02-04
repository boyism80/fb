function on_cast(me, spell)
    local mp = 100
    local sound = 8
    local effect = 2
    local buff_time = 21
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time)
        me:cover(true)
    end
end

function on_uncast(me, spell)
    me:cover(false)
end