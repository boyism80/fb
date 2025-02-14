function on_cast(me, spell)
    local mp = 90
    local sound = 7
    local effect = 2
    local buff_time = 625
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time)
    end
end

function on_buff(me, spell)

end

function on_unbuff(me, spell)

end