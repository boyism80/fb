function on_attack(ch, mob, damage)
    if ch:isbuff('투명') then
        ch:unbuff('투명')
        damage = damage * 2
    end

    if ch:state() == 0x05 then
        ch:state(0x00)
    end

    return damage
end