function on_cast(me, spell)
    local mp = 30
    local sound = 12
    local effect = 2
    local buff_time = 600
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time)
    end
end

function on_buff(me, spell)
    me:dam(me:dam()+3)
end

function on_unbuff(me, spell)
    me:dam(me:dam()-3)

end