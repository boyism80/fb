function handle_pickup(session)
    if session:state() == 0x05 then
        session:state(0x00)
        session:unbuff('����')
    end
end