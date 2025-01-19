function on_cast(me, you, spell)
    local mp = 80
    local sound = 71
    local effect = 14
    local buff_time = 10
    if debuff_cast(me, you, spell, mp, sound, effect) then
        you:buff(spell, buff_time)
        you:add_cc(CROWD_CONTROL_SIGHT)
    end
end

function on_uncast(me, spell)
    unbuff(me, spell)
    me:remove_cc(CROWD_CONTROL_SIGHT)
end

function on_concast(me, spell)

end

