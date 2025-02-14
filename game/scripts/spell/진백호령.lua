function on_cast(me, spell)
    local mp = 30
    local sound = 89
    local effect = 57
    local buff_time = 60

    if not buff_cast(me, me, spell, mp, sound, effect) then
        return
    end
    me:buff(spell, buff_time)
end

function on_buff(me, spell)
    me:skill_damage_rate(me:skill_damage_rate() + 500)
end

function on_unbuff(me, spell)
    me:skill_damage_rate(me:skill_damage_rate() - 500)

end