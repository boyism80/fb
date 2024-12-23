function on_pickup(ch)
    if ch:state() == 0x05 then
        ch:state(0x00)
        ch:unbuff('투명')
    end
end