function on_cast(me, spell)
    local mp = 30
    local sound = 7
    local effect = 136
    local buff_time = 180
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time, me)
    end
end

function on_buff(me, spell)

end

function on_unbuff(me, spell)

end