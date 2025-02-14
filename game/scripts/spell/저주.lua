function on_cast(me, you, spell)
    local mp = 30
    local sound = 69
    local effect = 13
    local buff_time = 185
    if debuff_cast(me, you, spell, mp, sound, effect) then
        you:buff(spell, buff_time, me)
    end
end

function on_buff(me, spell)
    me:buff_phydef(me:buff_phydef() + 30)
end

function on_unbuff(me, spell)
    me:buff_phydef(me:buff_phydef() - 30)

end