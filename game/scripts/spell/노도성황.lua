function on_cast(me, you, spell)
    local mp = 30
    local sound = 106
    local effect = 96
    local buff_time = 30
    if debuff_cast(me, you, spell, mp, sound, effect) then
        you:buff(spell, buff_time, me)
    end
end

function on_uncast(me, spell)
    unbuff(me, spell)
end

function on_concast(me, caster, buff)
    local damage = (me:base_hp() * 5) // 100
    me:damage(damage, caster)
    me:effect(96)
end