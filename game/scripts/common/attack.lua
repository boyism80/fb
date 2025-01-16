function on_attack(ch, mob, damage)
    if ch:isbuff('투명') then
        ch:unbuff('투명')
        damage = damage * 8
    end

    return damage
end