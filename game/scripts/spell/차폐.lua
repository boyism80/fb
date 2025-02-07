function on_cast(me, spell)
    local mp = 100
    local sound = 8
    local effect = 2
    local buff_time = 21
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time)
    end
end

function on_buff(me, spell)
    me:cover(true)
end

function on_unbuff(me, spell)
    me:cover(false)

end