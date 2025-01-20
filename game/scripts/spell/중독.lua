function on_cast(me, you, spell)
    local mp = 30
    local sound = 68
    local effect = 14
    local buff_time = 30
    if debuff_cast(me, you, spell, mp, sound, effect) then
        you:buff(spell, buff_time, me)
    end
end

function on_uncast(me, spell)
    unbuff(me, spell)
end

function on_concast(me, caster, buff)
    local damage = math.min((me:base_hp() * 5) // 100, 14994)
    if damage > me:hp() - 100 then
        damage = math.max(0, me:hp() - 100)
    end
    me:damage(damage, caster)
end