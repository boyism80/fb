function on_cast(me, spell)
    local mp = 30
    local sound = 78
    local effect = 11
    local buff_time = 600
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:detect(true)
        me:buff(spell, buff_time)
    end
end

function on_uncast(me, spell)
    me:detect(false)
    unbuff(me, spell)
end