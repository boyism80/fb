function on_cast(me, you, spell)
    local mp = 80
    local sound = 70
    local effect = 1
    local buff_time = 20
    if debuff_cast(me, you, spell, mp, sound, effect) then
        you:buff(spell, buff_time, me)
        you:paralysis(true)
    end
end

function on_uncast(me, spell)
    unbuff(me, spell)
    me:paralysis(false)
end