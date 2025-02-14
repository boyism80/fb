function on_cast(me, spell)
    local mp = 30
    local sound = 2
    local effect = nil
    local buff_time = 11
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time, me)
    end
end

function on_buff(me, spell)

end

function on_unbuff(me, spell)

end

function on_concast(me, caster, buff)
    me:effect(133)
    me:sound(2)
    me:mp_up(me:base_mp() // 10)
end