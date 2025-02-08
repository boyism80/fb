function on_attack(me, you)
    math.randomseed(seed())
    if math.random() > 0.01 then
        return false
    end
    
    if you == nil then
        return true
    end

    local ptr_me = me:ptr()
    local ptr_you = you:ptr()
    local thread = me:thread()

    me:chat('여의주의 힘을 받은 용이여..')
    sleep(1000);
    
    if not assert_alive(me) then
        return false
    end
    me:chat('그대 이름은 청룡일지다...')
    sleep(1000);
    
    if not assert_alive(me) then
        return false
    end
    me:chat('나 청의태자의 이름으로 말하노니...')
    sleep(1000);

    if not assert_alive(me) then
        return false
    end
    me:chat('네 포효를 적에게 발산하라!!')
    if assert_alive(you) and me:near(you) then
        me:cast(you, '청룡마령참')
    end

    sleep(1000);
    me:chat('청룡마령참!!', 0x03)
    
    sleep(1000);
    return true
end