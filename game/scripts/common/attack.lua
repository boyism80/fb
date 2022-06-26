function on_attack(session, mob)
    if session:state() == 0x05 then
        session:state(0x00)
        session:unbuff('투명')
    end
end