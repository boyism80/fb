function on_cast(me, spell)
    local mp = 150
    local sound = 9
    local effect = 72
    local buff_time = 600

    if not buff_cast(me, me, spell, mp, sound, effect) then
        return
    end
    me:unbuff('이중공격')
    me:unbuff('삼중공격')
    me:unbuff('오중공격')
    me:buff(spell, buff_time)
    me:damage_rate(me:damage_rate() + 3000)
end

function on_uncast(me, spell)
    me:damage_rate(me:damage_rate() - 3000)
end