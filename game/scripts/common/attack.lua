function handle_attack(session, mob)
    if session:state() == 0x05 then
        session:state(0x00)
        session:unbuff('≈ı∏Ì')
    end
end