function on_cast(me, you, spell)
    local mp = 30
    local sound = 8
    local effect = 110
    local buff_time = 180
    if buff_cast(me, you, spell, mp, sound, effect) then
        you:buff_phydef(you:buff_phydef() -10)
        you:buff(spell, buff_time, me)
    end
end

function on_uncast(me, spell)
    me:buff_phydef(me:buff_phydef() + 10)
    unbuff(me, spell)
end