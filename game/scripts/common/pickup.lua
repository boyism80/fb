function handle_pickup(session)
    if session:state() == 0x05 then
        session:state(0x00)
        session:unbuff('투명')
    end
end