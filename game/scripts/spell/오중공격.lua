function on_cast(me, spell)
    local mp = 200
    local sound = 9
    local effect = 72
    local buff_time = 600

    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time)
    end
end

function on_buff(me, spell)
    me:unbuff('이중공격')
    me:unbuff('삼중공격')
    me:unbuff('사중공격')
    me:damage_rate(me:damage_rate() + 4000)
end

function on_unbuff(me, spell)
    me:damage_rate(me:damage_rate() - 4000)

end