function on_cast(me, you, spell)
    local mp = 30
    local sound = 8
    local effect = 110
    local buff_time = 180
    if buff_cast(me, you, spell, mp, sound, effect) then
        you:buff(spell, buff_time, me)
    end
end

function on_buff(me, spell)
    me:damage_derate(me:damage_derate() + 1000)
end

function on_unbuff(me, spell)
    me:damage_derate(me:damage_derate() - 1000)

end