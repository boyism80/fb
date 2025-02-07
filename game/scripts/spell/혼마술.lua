function on_cast(me, you, spell)
    local mp = 40
    local sound = 79
    local effect = 39
    local buff_time = 425
    if debuff_cast(me, you, spell, mp, sound, effect) then
        you:buff(spell, buff_time, me)
    end
end

function on_buff(me, spell)
    me:buff_phydef(me:buff_phydef() + 50)
end

function on_unbuff(me, spell)
    me:buff_phydef(me:buff_phydef() - 50)

end