function on_cast(me, spell)
    local mp = 0
    local sound = 39
    local effect = 11
    local buff_time = 625
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time)
    end
end

function on_buff(me, spell)

end

function on_unbuff(me, spell)

end