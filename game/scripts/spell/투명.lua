function on_cast(me, spell)
    local mp = 0
    local sound = 28
    local effect = 20
    local buff_time = 180
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:state(0x05)
        me:buff(spell, buff_time)
    end
end

function on_uncast(me, spell)
    me:state(0x00)
    me:message('투명 해제')
end