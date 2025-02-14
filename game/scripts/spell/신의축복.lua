function on_cast(me, spell)
    local mp = 30
    local sound = 6
    local effect = 58
    local buff_time = 600
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time)
    end
end

function on_buff(me, spell)
    me:hit(me:hit()+3)
end

function on_unbuff(me, spell)
    me:hit(me:hit()-3)

end