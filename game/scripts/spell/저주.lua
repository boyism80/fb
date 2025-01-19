function on_cast(me, you, spell)
    local mp = 30
    local sound = 69
    local effect = 13
    local buff_time = 185
    if debuff_cast(me, you, spell, mp, sound, effect) then
        you:buff_phydef(you:buff_phydef() + 30)
        you:buff(spell, buff_time)
    end
end

function on_uncast(me, spell)
    me:buff_phydef(me:buff_phydef() - 30)
    unbuff(me, spell)
end